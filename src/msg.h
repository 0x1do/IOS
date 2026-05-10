#pragma once
#include "kernel.h"

#define MSG_MAGIC0      0x1D
#define MSG_MAGIC1      0x05
#define MSG_MAX_NODES   8
#define MSG_MAX_PAYLOAD 512

typedef struct __attribute__((packed)) {
	uint8_t  magic[2];
	uint8_t  sender;
	uint8_t  target;
	uint16_t length;
} MsgHeader;

extern uint8_t g_node_id;

void msg_set_id(uint8_t id);
void msg_send(uint8_t target, const char *text, int len);
void msg_poll(void);
