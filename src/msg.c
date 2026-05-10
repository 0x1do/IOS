#include "msg.h"
#include "connection.h"
#include "printk.h"
#include "mem.h"

uint8_t g_node_id = 0xFF;

static uint8_t checksum(const uint8_t *data, int len)
{
	uint8_t c = 0;
	for (int i = 0; i < len; i++)
		c ^= data[i];
	return c;
}

void msg_set_id(uint8_t id)
{
	if (id >= MSG_MAX_NODES) {
		printk("invalid id (0-%d)\n", MSG_MAX_NODES - 1);
		return;
	}
	g_node_id = id;
	printk("node id set to %d\n", id);

	uint8_t zero = 0;
	msg_send(id, (const char *)&zero, 1);
}

void msg_send(uint8_t target, const char *text, int len)
{
	if (g_node_id == 0xFF) {
		printk("set node id first (setid <id>)\n");
		return;
	}
	if (target >= MSG_MAX_NODES) {
		printk("invalid target (0-%d)\n", MSG_MAX_NODES - 1);
		return;
	}
	if (len <= 0 || len > MSG_MAX_PAYLOAD) {
		printk("message too long (max %d)\n", MSG_MAX_PAYLOAD);
		return;
	}

	uint8_t pkt[sizeof(MsgHeader) + MSG_MAX_PAYLOAD + 1];
	MsgHeader *hdr = (MsgHeader *)pkt;
	hdr->magic[0] = MSG_MAGIC0;
	hdr->magic[1] = MSG_MAGIC1;
	hdr->sender   = g_node_id;
	hdr->target   = target;
	hdr->length   = (uint16_t)len;

	memcpy(pkt + sizeof(MsgHeader), text, len);

	int body_len = sizeof(MsgHeader) + len;
	pkt[body_len] = checksum(pkt, body_len);

	serial_write_raw(COM1_BASE, pkt, body_len + 1);
}

enum { RX_IDLE, RX_MAGIC1, RX_HDR, RX_DATA, RX_CHK };

static int     rx_state = RX_IDLE;
static uint8_t rx_buf[sizeof(MsgHeader) + MSG_MAX_PAYLOAD + 1];
static int     rx_pos  = 0;
static int     rx_need = 0;

void msg_poll(void)
{
	char c;
	while (serial_try_getc(COM1_BASE, &c)) {
		uint8_t b = (uint8_t)c;

		switch (rx_state) {
		case RX_IDLE:
			if (b == MSG_MAGIC0) {
				rx_buf[0] = b;
				rx_pos = 1;
				rx_state = RX_MAGIC1;
			}
			break;

		case RX_MAGIC1:
			if (b == MSG_MAGIC1) {
				rx_buf[1] = b;
				rx_pos = 2;
				rx_state = RX_HDR;
			} else {
				rx_state = RX_IDLE;
			}
			break;

		case RX_HDR:
			rx_buf[rx_pos++] = b;
			if (rx_pos >= (int)sizeof(MsgHeader)) {
				MsgHeader *h = (MsgHeader *)rx_buf;
				rx_need = h->length;
				if (rx_need > MSG_MAX_PAYLOAD || rx_need == 0)
					rx_state = RX_IDLE;
				else
					rx_state = RX_DATA;
			}
			break;

		case RX_DATA:
			rx_buf[rx_pos++] = b;
			if (rx_pos >= (int)sizeof(MsgHeader) + rx_need)
				rx_state = RX_CHK;
			break;

		case RX_CHK: {
			int body_len = sizeof(MsgHeader) + rx_need;
			if (checksum(rx_buf, body_len) == b) {
				MsgHeader *h = (MsgHeader *)rx_buf;
				if (h->target == g_node_id) {
					uint8_t *pl = rx_buf + sizeof(MsgHeader);
					pl[h->length] = '\0';
					printk("\n[msg from %d]: %s\n",
					       h->sender, (char *)pl);
				}
			}
			rx_state = RX_IDLE;
			break;
		}
		}
	}
}
