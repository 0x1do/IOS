#define _GNU_SOURCE
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#define MAGIC0       0x1D
#define MAGIC1       0x05
#define HDR_LEN      6
#define MAX_PAYLOAD  512
#define MAX_FRAME    (HDR_LEN + MAX_PAYLOAD + 1)
#define BROADCAST    0xFF
#define BUF_CAP      8192
#define MAX_PEERS    64
#define LISTEN_BACKLOG 16

typedef struct {
    int      fd;
    int      have_node;
    uint8_t  node_id;
    size_t   buf_len;
    uint8_t  buf[BUF_CAP];
} Peer;

static Peer     peers[MAX_PEERS];
static int      verbose = 0;
static char     sock_path[256] = "/tmp/ioslink.sock";
static int      server_fd = -1;

static void vlog(const char *fmt, ...) {
    if (!verbose) return;
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[hub] ");
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    fflush(stderr);
    va_end(ap);
}

static void cleanup(void) {
    if (server_fd >= 0) close(server_fd);
    unlink(sock_path);
}

static void on_signal(int sig) {
    (void)sig;
    cleanup();
    _exit(0);
}

static int peer_slot_alloc(int fd) {
    for (int i = 0; i < MAX_PEERS; i++) {
        if (peers[i].fd < 0) {
            peers[i].fd = fd;
            peers[i].have_node = 0;
            peers[i].node_id = 0;
            peers[i].buf_len = 0;
            return i;
        }
    }
    return -1;
}

static int peer_find_by_node(uint8_t node_id) {
    for (int i = 0; i < MAX_PEERS; i++) {
        if (peers[i].fd >= 0 && peers[i].have_node && peers[i].node_id == node_id)
            return i;
    }
    return -1;
}

static void peer_drop(int idx) {
    if (idx < 0 || peers[idx].fd < 0) return;
    int fd = peers[idx].fd;
    int node = peers[idx].have_node ? peers[idx].node_id : -1;
    close(fd);
    peers[idx].fd = -1;
    peers[idx].have_node = 0;
    peers[idx].buf_len = 0;
    vlog("-conn fd=%d node=%d", fd, node);
}

static int send_all(int fd, const uint8_t *buf, size_t len) {
    while (len > 0) {
        ssize_t n = send(fd, buf, len, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}

static void process_peer_buffer(int idx) {
    Peer *p = &peers[idx];

    for (;;) {
        size_t i = 0;
        while (i + 1 < p->buf_len &&
               !(p->buf[i] == MAGIC0 && p->buf[i + 1] == MAGIC1)) {
            i++;
        }
        if (i > 0) {
            memmove(p->buf, p->buf + i, p->buf_len - i);
            p->buf_len -= i;
        }
        if (p->buf_len < HDR_LEN) return;

        uint8_t  sender = p->buf[2];
        uint8_t  target = p->buf[3];
        uint16_t length = (uint16_t)p->buf[4] | ((uint16_t)p->buf[5] << 8);

        if (length == 0 || length > MAX_PAYLOAD) {
            memmove(p->buf, p->buf + 1, p->buf_len - 1);
            p->buf_len -= 1;
            continue;
        }

        size_t total = (size_t)HDR_LEN + length + 1;
        if (p->buf_len < total) return;

        uint8_t chk = 0;
        for (size_t k = 0; k < (size_t)HDR_LEN + length; k++)
            chk ^= p->buf[k];
        if (chk != p->buf[HDR_LEN + length]) {
            memmove(p->buf, p->buf + 1, p->buf_len - 1);
            p->buf_len -= 1;
            continue;
        }

        uint8_t frame[MAX_FRAME];
        memcpy(frame, p->buf, total);

        if (!p->have_node || p->node_id != sender) {
            int prev = peer_find_by_node(sender);
            if (prev >= 0 && prev != idx) {
                vlog("reassign node=%u from fd=%d to fd=%d",
                     sender, peers[prev].fd, p->fd);
                peers[prev].have_node = 0;
            }
            p->have_node = 1;
            p->node_id = sender;
            vlog("learn node=%u fd=%d", sender, p->fd);
        }

        if (target == sender) {
            vlog("register node=%u", sender);
        } else if (target == BROADCAST) {
            for (int j = 0; j < MAX_PEERS; j++) {
                if (peers[j].fd < 0 || j == idx) continue;
                if (send_all(peers[j].fd, frame, total) < 0) {
                    peer_drop(j);
                }
            }
            vlog("bcast %u->* len=%zu", sender, total);
        } else {
            int dst = peer_find_by_node(target);
            if (dst < 0) {
                vlog("drop %u->%u (unknown target)", sender, target);
            } else if (send_all(peers[dst].fd, frame, total) < 0) {
                peer_drop(dst);
            } else {
                vlog("route %u->%u len=%zu", sender, target, total);
            }
        }

        memmove(p->buf, p->buf + total, p->buf_len - total);
        p->buf_len -= total;
    }
}

static void usage(const char *prog) {
    fprintf(stderr,
            "usage: %s [--sock PATH] [-v|--verbose]\n"
            "  --sock PATH    Unix socket path (default: /tmp/ioslink.sock)\n"
            "  -v, --verbose  log routing decisions to stderr\n",
            prog);
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "--sock")) {
            if (i + 1 >= argc) { usage(argv[0]); return 2; }
            snprintf(sock_path, sizeof sock_path, "%s", argv[++i]);
        } else if (!strcmp(a, "-v") || !strcmp(a, "--verbose")) {
            verbose = 1;
        } else if (!strcmp(a, "-h") || !strcmp(a, "--help")) {
            usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "unknown argument: %s\n", a);
            usage(argv[0]);
            return 2;
        }
    }

    for (int i = 0; i < MAX_PEERS; i++) peers[i].fd = -1;

    unlink(sock_path);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    if (strlen(sock_path) >= sizeof addr.sun_path) {
        fprintf(stderr, "socket path too long\n");
        return 1;
    }
    strcpy(addr.sun_path, sock_path);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind");
        return 1;
    }
    if (listen(server_fd, LISTEN_BACKLOG) < 0) {
        perror("listen");
        cleanup();
        return 1;
    }
    chmod(sock_path, 0666);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGHUP,  on_signal);

    vlog("listening on %s", sock_path);

    struct pollfd pfds[MAX_PEERS + 1];

    for (;;) {
        int nfds = 0;
        pfds[nfds].fd = server_fd;
        pfds[nfds].events = POLLIN;
        nfds++;

        int peer_idx[MAX_PEERS];
        for (int i = 0; i < MAX_PEERS; i++) {
            if (peers[i].fd < 0) continue;
            peer_idx[nfds - 1] = i;
            pfds[nfds].fd = peers[i].fd;
            pfds[nfds].events = POLLIN;
            nfds++;
        }

        int r = poll(pfds, (nfds_t)nfds, -1);
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        if (pfds[0].revents & POLLIN) {
            int c = accept(server_fd, NULL, NULL);
            if (c >= 0) {
                int slot = peer_slot_alloc(c);
                if (slot < 0) {
                    fprintf(stderr, "[hub] peer table full, rejecting fd=%d\n", c);
                    close(c);
                } else {
                    vlog("+conn fd=%d", c);
                }
            }
        }

        for (int k = 1; k < nfds; k++) {
            int idx = peer_idx[k - 1];
            if (peers[idx].fd < 0) continue;
            if (!(pfds[k].revents & (POLLIN | POLLHUP | POLLERR))) continue;

            Peer *p = &peers[idx];
            size_t avail = BUF_CAP - p->buf_len;
            if (avail == 0) {
                fprintf(stderr, "[hub] buffer full on fd=%d, dropping\n", p->fd);
                peer_drop(idx);
                continue;
            }

            ssize_t n = recv(p->fd, p->buf + p->buf_len, avail, 0);
            if (n <= 0) {
                if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
                peer_drop(idx);
                continue;
            }
            p->buf_len += (size_t)n;
            process_peer_buffer(idx);
        }
    }

    cleanup();
    return 0;
}
