#pragma once
#include "kernel.h"
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEY_UP    256
#define KEY_DOWN  257
#define KEY_LEFT  258
#define KEY_RIGHT 259
#define KEY_HOME  260
#define KEY_END   261
#define KEY_DEL   262

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
uint8_t readKeyData();
void clearKeyboardBuffer();
char codeToChar(uint8_t scan_code);
void printScanCode(uint8_t scan_code);
void keyboardPoll();
char getChar();
int scank(const char *format, ...);
int sscank(const char *input, const char *fmt, ...);
int fgetsk(char *buf, int size);
int getCharEx(void);
