#pragma once
#include "kernel.h"

#define COM1_BASE 0x3F8

#define UART_RBR 0
#define UART_THR 0
#define UART_DLL 0
#define UART_IER 1
#define UART_DLM 1
#define UART_IIR 2
#define UART_FCR 2
#define UART_LCR 3
#define UART_MCR 4
#define UART_LSR 5

#define LSR_DATA_READY (1u << 0)
#define LSR_THR_EMPTY (1u << 5)


static inline uint8_t uart_read(uint16_t base, uint8_t reg);
static inline void uart_write(uint16_t base, uint8_t reg, uint8_t val);
void serial_init(uint16_t base, uint16_t baud_divisor);
static inline int serial_rx_ready(uint16_t base);
static inline int serial_tx_ready(uint16_t base);
void serial_putc(uint16_t base, char c);
void serial_write(uint16_t base, const char *s);
void serial_write_raw(uint16_t base, const uint8_t *data, int len);
int serial_try_getc(uint16_t base, char *out);
char serial_getc_blocking(uint16_t base);\
char getCharSerial(void);
char getCharAny(void);
void enterConn();
