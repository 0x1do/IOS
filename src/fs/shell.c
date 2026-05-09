#include "shell.h"
#include "allocator.h"
#include "connection.h"
#include "disksim.h"
#include "fs.h"
#include "keyboard.h"
#include "mem.h"
#include "msg.h"
#include "printk.h"
#include "string.h"

#define SECTOR_SIZE 512

#define COND_MOUNT 0x01
#define COND_UMOUNT 0x02

typedef struct {
	char *name;
	int (*handler)(int, char **);
	char conditions;
} Command;

extern void shellRegisterFilesystem(ShellFilesystem *);
extern void printkBySel(DiskOperations *disk,
						ShellFsOperations *fsOprs,
						const ShellEntry *parent,
						ShellEntry *entry,
						const char *name,
						int sel,
						int num);
void do_shell(void);
void unknown_command(void);
int separateString(char *buf, char *ptrs[]);
double get_percentage(unsigned int number, unsigned int total);

int shell_cmd_format(int argc, char *argv[]);
int shell_cmd_exit(int argc, char *argv[]);
int shell_cmd_mount(int argc, char *argv[]);
int shell_cmd_touch(int argc, char *argv[]);
int shell_cmd_cd(int argc, char *argv[]);
int shell_cmd_ls(int argc, char *argv[]);
int shell_cmd_mkdir(int argc, char *argv[]);
int shell_cmd_fill(int argc, char *argv[]);
int shell_cmd_rm(int argc, char *argv[]);
int shell_cmd_cat(int argc, char *argv[]);
int shell_cmd_rmdir(int argc, char *argv[]);
int shell_cmd_clear(int argc, char *argv[]);
int shell_cmd_echo(int argc, char *argv[]);
int shell_cmd_conn(int argc, char *argv[]);
int shell_cmd_setid(int argc, char *argv[]);
int shell_cmd_sendmsg(int argc, char *argv[]);

static Command g_commands[] = { { "cd", shell_cmd_cd, COND_MOUNT },
								{ "mount", shell_cmd_mount, COND_UMOUNT },
								{ "touch", shell_cmd_touch, COND_MOUNT },
								{ "fill", shell_cmd_fill, COND_MOUNT },
								{ "ls", shell_cmd_ls, COND_MOUNT },
								{ "format", shell_cmd_format, COND_UMOUNT },
								{ "mkdir", shell_cmd_mkdir, COND_MOUNT },
								{ "rm", shell_cmd_rm, COND_MOUNT },
								{ "rmdir", shell_cmd_rmdir, COND_MOUNT },
								{ "exit", shell_cmd_exit, 0 },
								{ "cat", shell_cmd_cat, COND_MOUNT },
								{ "clear", shell_cmd_clear, 0 },
								{ "echo", shell_cmd_echo, 0 },
								{ "conn", shell_cmd_conn, 0 },
								{ "setid", shell_cmd_setid, 0 },
								{ "sendmsg", shell_cmd_sendmsg, 0 } };

static ShellFilesystem g_fs;
static ShellFsOperations g_fsOprs;
static ShellEntry g_rootDir;
static ShellEntry g_currentDir;
static DiskOperations g_disk;

static ShellEntry path[256];
static int pathTop = 0;

int g_commandsCount = sizeof(g_commands) / sizeof(Command);
int g_isMounted;

#define HIST_MAX 16
#define HIST_CMDLEN 1000

static char hist_buf[HIST_MAX][HIST_CMDLEN];
static int hist_total = 0;

/* Output redirection - set by do_shell before command dispatch. */
static char *g_redir_file = NULL;

static int is_help(int argc, char *argv[])
{
	return argc == 2 &&
		(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0);
}

static int write_string_to_file(const char *filename, const char *data, int len)
{
	ShellEntry entry;
	int result;

	result =
		g_fsOprs.lookup(&g_disk, &g_fsOprs, &g_currentDir, &entry, filename);
	if (result) {
		result = g_fsOprs.fileOprs->create(
			&g_disk, &g_fsOprs, &g_currentDir, filename, &entry);
		if (result) {
			printk("cannot create %s\n", filename);
			return -1;
		}
	}

	g_fsOprs.fileOprs->write(
		&g_disk, &g_fsOprs, &g_currentDir, &entry, 0, len, data);

	return 0;
}

static void redraw_tail(const char *buf, int cur, int len, int blank)
{
	for (int j = cur; j < len; j++)
		putChar(buf[j]);
	if (blank)
		putChar(' ');
	int back = len - cur + blank;
	for (int j = 0; j < back; j++)
		putChar('\b');
}

static void erase_line(const char *buf, int cur, int len)
{
	/* move cursor to end */
	for (int j = cur; j < len; j++)
		putChar(buf[j]);
	/* erase backwards */
	for (int j = 0; j < len; j++) {
		putChar('\b');
		putChar(' ');
		putChar('\b');
	}
}

static int shell_readline(char *buf, int size)
{
	int len = 0;
	int cur = 0;
	int browse = hist_total;
	char saved[HIST_CMDLEN];
	int saved_len = 0;
	int ch;

	saved[0] = '\0';

	while (len < size - 1) {
		ch = getCharEx();

		if (ch == '\n') {
			putChar('\n');
			break;
		}

		if (ch == '\b') {
			if (cur > 0) {
				cur--;
				len--;
				for (int j = cur; j < len; j++)
					buf[j] = buf[j + 1];
				putChar('\b');
				redraw_tail(buf, cur, len, 1);
			}
			continue;
		}

		if (ch == KEY_DEL) {
			if (cur < len) {
				len--;
				for (int j = cur; j < len; j++)
					buf[j] = buf[j + 1];
				redraw_tail(buf, cur, len, 1);
			}
			continue;
		}

		if (ch == KEY_LEFT) {
			if (cur > 0) {
				cur--;
				putChar('\b');
			}
			continue;
		}

		if (ch == KEY_RIGHT) {
			if (cur < len) {
				putChar(buf[cur]);
				cur++;
			}
			continue;
		}

		if (ch == KEY_HOME) {
			while (cur > 0) {
				cur--;
				putChar('\b');
			}
			continue;
		}

		if (ch == KEY_END) {
			while (cur < len) {
				putChar(buf[cur]);
				cur++;
			}
			continue;
		}

		if (ch == KEY_UP || ch == KEY_DOWN) {
			int oldest = hist_total - HIST_MAX;
			if (oldest < 0)
				oldest = 0;

			if (ch == KEY_UP) {
				if (browse <= oldest)
					continue;
				if (browse == hist_total) {
					memcpy(saved, buf, len);
					saved[len] = '\0';
					saved_len = len;
				}
				browse--;
			} else {
				if (browse >= hist_total)
					continue;
				browse++;
			}

			erase_line(buf, cur, len);

			const char *src;
			int slen;
			if (browse == hist_total) {
				src = saved;
				slen = saved_len;
			} else {
				src = hist_buf[browse % HIST_MAX];
				slen = strlen(src);
			}
			if (slen >= size)
				slen = size - 1;
			memcpy(buf, src, slen);
			len = slen;
			cur = slen;

			for (int j = 0; j < len; j++)
				putChar(buf[j]);

			continue;
		}

		if (ch > 127)
			continue;

		/* insert character at cursor */
		if (len >= size - 1)
			continue;
		for (int j = len; j > cur; j--)
			buf[j] = buf[j - 1];
		buf[cur] = (char)ch;
		len++;
		cur++;
		/* print from inserted char to end, reposition cursor */
		for (int j = cur - 1; j < len; j++)
			putChar(buf[j]);
		for (int j = 0; j < len - cur; j++)
			putChar('\b');
	}

	buf[len] = '\0';

	if (len > 0) {
		memcpy(hist_buf[hist_total % HIST_MAX], buf, len + 1);
		hist_total++;
	}

	return len;
}

int initFs()
{
	if (disksimInit(NUMBER_OF_SECTORS, SECTOR_SIZE, &g_disk) < 0) {
		printk("disk simulator initialization has been failed\n");
		return -1;
	}

	shellRegisterFilesystem(&g_fs);

	do_shell();

	return 0;
}

void do_shell(void)
{
	char buf[1000];
	char command[100];
	char *argv[100];
	int argc;
	int i, j = 0;

	printk("%s File system shell\n", g_fs.name);

	while (-1) {
		msg_poll();
		printk("[user/%s]# ", g_currentDir.name);
		shell_readline(buf, sizeof(buf));

		argc = separateString(buf, argv);
		if (argc == 0)
			continue;

		g_redir_file = NULL;
		for (int k = 0; k < argc; k++) {
			if (strcmp(argv[k], ">") == 0) {
				if (k + 1 < argc) {
					g_redir_file = argv[k + 1];
					argc = k;
				} else {
					printk("syntax error: missing filename after >\n");
					argc = 0;
				}
				break;
			}
		}
		if (argc == 0)
			continue;

		for (i = 0; i < g_commandsCount; i++) {
			if (strcmp(g_commands[i].name, argv[0]) == 0) {
				if (is_help(argc, argv) ||
					checkConditions(g_commands[i].conditions) == 0)
					g_commands[i].handler(argc, argv);
				break;
			}
		}

		g_redir_file = NULL;

		if (argc != 0 && i == g_commandsCount)
			unknown_command();
	}
}

int shell_cmd_cd(int argc, char *argv[])
{
	ShellEntry newEntry;
	int result, i;

	if (is_help(argc, argv)) {
		printk("cd [directory] - change working directory\n");
		printk("  cd       go to root\n");
		printk("  cd ..    go up one level\n");
		return 0;
	}

	path[0] = g_rootDir;

	if (argc > 2) {
		printk("usage : %s [directory]\n", argv[0]);
		return 0;
	}

	if (argc == 1)
		pathTop = 0;
	else {
		if (strcmp(argv[1], ".") == 0)
			return 0;
		else if (strcmp(argv[1], "..") == 0 && pathTop > 0)
			pathTop--;
		else {
			result = g_fsOprs.lookup(
				&g_disk, &g_fsOprs, &g_currentDir, &newEntry, argv[1]);

			if (result) {
				printk("directory not found\n");
				return -1;
			} else if (!newEntry.isDirectory) {
				printk("%s is not a directory\n", argv[1]);
				return -1;
			}
			path[++pathTop] = newEntry;
		}
	}

	g_currentDir = path[pathTop];

	return 0;
}

int shell_cmd_exit(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("exit - unmount filesystem and shut down\n");
		return 0;
	}
	disksimUninit(&g_disk);
	_exit(0);

	return 0;
}

int shell_cmd_mount(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("mount - mount the filesystem\n");
		return 0;
	}
	int result;

	if (g_fs.mount == NULL) {
		printk("The mount functions is NULL\n");
		return 0;
	}

	result = g_fs.mount(&g_disk, &g_fsOprs, &g_rootDir);
	g_currentDir = g_rootDir;

	if (result < 0) {
		printk("%s file system mounting has been failed\n", g_fs.name);
		return -1;
	} else {
		printk("%s file system has been mounted successfully\n", g_fs.name);
		g_isMounted = 1;
	}

	return 0;
}

int shell_cmd_umount(int argc, char *argv[])
{
	g_isMounted = 0;

	if (g_fs.umount == NULL)
		return 0;

	g_fs.umount(&g_disk, &g_fsOprs);
	return 0;
}

int shell_cmd_touch(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("touch <file> - create an empty file\n");
		return 0;
	}
	ShellEntry entry;
	int result;
	if (argc < 2) {
		printk("usage : touch [files...]\n");
		return -2;
	}
	result = g_fsOprs.fileOprs->create(
		&g_disk, &g_fsOprs, &g_currentDir, argv[1], &entry);

	if (result) {
		printk("create failed\n");
		return -1;
	}

	return 0;
}

int shell_cmd_fill(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk(
			"fill <file> <size> - write <size> bytes of test data to a file\n");
		return 0;
	}
	ShellEntry entry;
	char *buffer;
	int size;
	int result;

	if (argc != 3) {
		printk("usage : fill [file] [size]\n");
		return 0;
	}

	size = atoi(argv[2]);
	if (size <= 0) {
		printk("invalid size\n");
		return -1;
	}

	result =
		g_fsOprs.lookup(&g_disk, &g_fsOprs, &g_currentDir, &entry, argv[1]);
	if (result) {
		result = g_fsOprs.fileOprs->create(
			&g_disk, &g_fsOprs, &g_currentDir, argv[1], &entry);
		if (result) {
			printk("create failed\n");
			return -1;
		}
	}

	buffer = (char *)kmalloc(size);
	if (!buffer) {
		printk("out of memory\n");
		return -1;
	}

	memset(buffer, 'A', size);

	g_fsOprs.fileOprs->write(
		&g_disk, &g_fsOprs, &g_currentDir, &entry, 0, size, buffer);

	kfree(buffer);

	return 0;
}

int shell_cmd_rm(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("rm <files...> - remove one or more files\n");
		return 0;
	}
	int i;

	if (argc < 2) {
		printk("usage : rm [files...]\n");
		return 0;
	}

	for (i = 1; i < argc; i++) {
		if (g_fsOprs.fileOprs->remove(
				&g_disk, &g_fsOprs, &g_currentDir, argv[i]))
			printk("cannot remove file\n");
	}

	return 0;
}

int shell_cmd_format(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("format - format the disk\n");
		return 0;
	}
	int result;
	unsigned int k = 0;
	char *param = NULL;

	if (argc >= 2)
		param = argv[1];

	result = g_fs.format(&g_disk);

	if (result < 0) {
		printk("%s formatting is failed\n", g_fs.name);
		return -1;
	}

	printk("disk has been formatted successfully\n");
	return 0;
}

int shell_cmd_df(int argc, char *argv[])
{
	unsigned int used, total;
	int result;

	g_fsOprs.stat(&g_disk, &g_fsOprs, &total, &used);

	printk(
		"free sectors : %u(%.2lf%%)\tused sectors : %u(%.2lf%%)\ttotal : %u\n",
		total - used,
		get_percentage(total - used, g_disk.numberOfSectors),
		used,
		get_percentage(used, g_disk.numberOfSectors),
		total);

	return 0;
}

int shell_cmd_mkdir(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("mkdir <name> - create a directory\n");
		return 0;
	}
	ShellEntry entry;
	int result;

	if (argc != 2) {
		printk("usage : %s [name]\n", argv[0]);
		return 0;
	}

	result = g_fsOprs.mkdir(&g_disk, &g_fsOprs, &g_currentDir, argv[1], &entry);

	if (result) {
		printk("cannot create directory\n");
		return -1;
	}

	return 0;
}

int shell_cmd_rmdir(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("rmdir <name> - remove a directory\n");
		return 0;
	}
	int result;

	if (argc != 2) {
		printk("usage : %s [name]\n", argv[0]);
		return 0;
	}

	result = g_fsOprs.rmdir(&g_disk, &g_fsOprs, &g_currentDir, argv[1]);

	if (result) {
		printk("cannot remove directory\n");
		return -1;
	}

	return 0;
}

int shell_cmd_mkdirst(int argc, char *argv[])
{
	ShellEntry entry;
	int result, i, count;
	char buf[10];

	if (argc != 2) {
		printk("usage : %s [count]\n", argv[0]);
		return 0;
	}

	sscank(argv[1], "%d", &count);
	for (i = 0; i < count; i++) {
		printk(buf, "%d", i);
		result = g_fsOprs.mkdir(&g_disk, &g_fsOprs, &g_currentDir, buf, &entry);

		if (result) {
			printk("cannot create directory\n");
			return -1;
		}
	}

	return 0;
}

int shell_cmd_cat(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("cat <file> - display file contents\n");
		return 0;
	}
	ShellEntry entry;
	char buf[1025] = {
		0,
	};
	int result;
	unsigned long offset = 0;

	if (argc != 2) {
		printk("usage : %s [file name]\n", argv[0]);
		return 0;
	}

	result =
		g_fsOprs.lookup(&g_disk, &g_fsOprs, &g_currentDir, &entry, argv[1]);
	if (result) {
		printk("%s lookup failed\n", argv[1]);
		return -1;
	}

	while (
		(result = (g_fsOprs.fileOprs->read(
			 &g_disk, &g_fsOprs, &g_currentDir, &entry, offset, 1024, buf))) >
		0) {
		printk("%s", buf);
		offset += 1024;
		memset(buf, 0, sizeof(buf));
	}
	printk("\n");
}

int shell_cmd_clear(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("clear - clear the screen\n");
		return 0;
	}
	printk("\033[2J\033[H");
	return 0;
}

int shell_cmd_echo(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("echo <text...> - print text (supports > redirect)\n");
		return 0;
	}
	char output[1000];
	int pos = 0;

	for (int i = 1; i < argc; i++) {
		int slen = strlen(argv[i]);
		if (pos + slen >= (int)sizeof(output) - 1)
			break;
		memcpy(output + pos, argv[i], slen);
		pos += slen;
		if (i < argc - 1)
			output[pos++] = ' ';
	}
	output[pos] = '\0';

	if (g_redir_file) {
		if (!g_isMounted) {
			printk("file system is not mounted\n");
			return -1;
		}
		return write_string_to_file(g_redir_file, output, pos);
	}

	printk("%s\n", output);
	return 0;
}

int shell_cmd_conn(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("conn - enter serial link mode (Ctrl+C to exit)\n");
		return 0;
	}
	printk("[conn] entering serial link mode...\n");
	enterConn();
	printk("[conn] exited serial link mode\n");
	return 0;
}

int shell_cmd_setid(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk(
			"setid <id> - set this node's ID (0-7) for encrypted messaging\n");
		return 0;
	}
	if (argc != 2) {
		printk("usage: setid <id>\n");
		return -1;
	}
	int id = atoi(argv[1]);
	msg_set_id((uint8_t)id);
	return 0;
}

int shell_cmd_sendmsg(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("sendmsg <target_id> <message...> - send encrypted message to a "
			   "node\n");
		printk("  requires setid first. only the target can decrypt.\n");
		return 0;
	}
	if (argc < 3) {
		printk("usage: sendmsg <target_id> <message...>\n");
		return -1;
	}

	int target = atoi(argv[1]);
	char msg[512];
	int pos = 0;

	for (int i = 2; i < argc && pos < 511; i++) {
		int slen = strlen(argv[i]);
		if (pos + slen >= 511)
			break;
		memcpy(msg + pos, argv[i], slen);
		pos += slen;
		if (i < argc - 1 && pos < 511)
			msg[pos++] = ' ';
	}
	msg[pos] = '\0';

	msg_send((uint8_t)target, msg, pos);
	printk("sent encrypted message to node %d\n", target);
	return 0;
}

int shell_cmd_ls(int argc, char *argv[])
{
	if (is_help(argc, argv)) {
		printk("ls [-l] - list directory contents\n");
		printk("  -l  long format (type, size, name)\n");
		return 0;
	}
	ShellEntryList list;
	ShellEntryListItem *current;
	int long_fmt = 0;

	if (argc >= 2 && strcmp(argv[1], "-l") == 0)
		long_fmt = 1;

	initEntryList(&list);
	if (g_fsOprs.readDir(&g_disk, &g_fsOprs, &g_currentDir, &list)) {
		printk("Failed to read_dir\n");
		return -1;
	}

	current = list.first;

	if (long_fmt) {
		printk("total %d\n", list.count);
		while (current) {
			if (current->entry.isDirectory)
				printk("d---\t%d\t\033[34m%s\033[0m\n",
					   current->entry.size,
					   current->entry.name);
			else
				printk(
					"----\t%d\t%s\n", current->entry.size, current->entry.name);
			current = current->next;
		}
	} else {
		while (current) {
			if (current->entry.isDirectory)
				printk("\033[34m%s\033[0m  ", current->entry.name);
			else
				printk("%s  ", current->entry.name);
			current = current->next;
		}
		printk("\n");
	}

	releaseEntryList(&list);
	return 0;
}

int checkConditions(int conditions)
{
	if (conditions & COND_MOUNT && !g_isMounted) {
		printk("file system is not mounted\n");
		return -1;
	}

	if (conditions & COND_UMOUNT && g_isMounted) {
		printk("file system is already mounted\n");
		return -1;
	}

	return 0;
}

void unknown_command(void)
{
	int i;

	printk(" * ");
	for (i = 0; i < g_commandsCount; i++) {
		if (i < g_commandsCount - 1)
			printk("%s, ", g_commands[i].name);
		else
			printk("%s", g_commands[i].name);
	}
	printk("\n");
}

int separateString(char *buf, char *ptrs[])
{
	int count = 0;

	while (*buf) {
		while (*buf && isspace(*buf))
			buf++;
		if (!*buf)
			break;

		if (*buf == '"') {
			buf++;
			ptrs[count++] = buf;
			while (*buf && *buf != '"')
				buf++;
			if (*buf == '"')
				*buf++ = '\0';
		} else {
			ptrs[count++] = buf;
			while (*buf && !isspace(*buf))
				buf++;
			if (*buf)
				*buf++ = '\0';
		}
	}

	return count;
}

double get_percentage(unsigned int number, unsigned int total)
{
	return ((double)number) / total * 100.;
}