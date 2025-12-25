#pragma once
#include "kernel.h"
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

uint8_t inb(uint16_t port);
uint8_t readKeyData();
void printScanCode(uint8_t scan_code);
void keyboardPoll();
char getChar();
void scanf(const char *format, ...);
