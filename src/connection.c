#include "connection.h"
#include "framework/printk.h"
#include "keyboard.h"

static inline uint8_t uart_read(uint16_t base, uint8_t reg)
{
	return inb((uint16_t)(base + reg));
}

static inline void uart_write(uint16_t base, uint8_t reg, uint8_t val)
{
	outb((uint16_t)(base + reg), val);
}

void serial_init(uint16_t base, uint16_t baud_divisor)
{
	uart_write(base, UART_IER, 0x00);
	uart_write(base, UART_LCR, 0x80);

	uart_write(base, UART_DLL, (uint8_t)(baud_divisor & 0xFF));
	uart_write(base, UART_DLM, (uint8_t)((baud_divisor >> 8) & 0xFF));

	uart_write(base, UART_LCR, 0x03);

	uart_write(base, UART_FCR, 0xC7);

	uart_write(base, UART_MCR, 0x0B);
}

static inline int serial_rx_ready(uint16_t base)
{
	return (uart_read(base, UART_LSR) & LSR_DATA_READY) != 0;
}

static inline int serial_tx_ready(uint16_t base)
{
	return (uart_read(base, UART_LSR) & LSR_THR_EMPTY) != 0;
}

void serial_putc(uint16_t base, char c)
{
	if (c == '\n') {
		serial_putc(base, '\r');
	}

	while (!serial_tx_ready(base)) {
		;;
	}
	uart_write(base, UART_THR, (uint8_t)c);
}

void serial_write(uint16_t base, const char *s)
{
	while (s && *s) {
		serial_putc(base, *s++);
	}
}

void serial_write_raw(uint16_t base, const uint8_t *data, int len)
{
	for (int i = 0; i < len; i++) {
		while (!serial_tx_ready(base)) {
			;;
		}
		uart_write(base, UART_THR, data[i]);
	}
}

int serial_try_getc(uint16_t base, char *out)
{
	if (!serial_rx_ready(base)) {
		return 0;
	}
	if (out) {
		*out = (char)uart_read(base, UART_RBR);
	} else {
		(void)uart_read(base, UART_RBR);
	}
	return 1;
}

char serial_getc_blocking(uint16_t base)
{
	while (!serial_rx_ready(base)) {
		;;
	}
	return (char)uart_read(base, UART_RBR);
}

char getCharSerial(void)
{
	char c = serial_getc_blocking(COM1_BASE);
	putChar(c);
	return c;
}

char getCharAny(void)
{
	while (1) {
		char sc;
		if (serial_try_getc(COM1_BASE, &sc)) {
			putChar(sc);
			return sc;
		}

		if ((inb(KEYBOARD_STATUS_PORT) & 0x01) != 0) {
			uint8_t code = inb(KEYBOARD_DATA_PORT);
			char ch = codeToChar(code);
			if (ch) {
				putChar(ch);
				return ch;
			}
		}
	}
}

void enterConn()
{
	int ctrl = 0;
	for (;;) {
		if ((inb(KEYBOARD_STATUS_PORT) & 0x01) != 0) {
			uint8_t code = inb(KEYBOARD_DATA_PORT);
			if (code == 0x1D) { ctrl = 1; continue; }
			if (code == 0x9D) { ctrl = 0; continue; }
			if (ctrl && code == 0x2E) return; /* Ctrl+C exits */
			if (code & 0x80) continue;
			char ch = codeToChar(code);
			if (ch) {
				putChar(ch);
				serial_putc(COM1_BASE, ch);
			}
		}

		char r;
		if (serial_try_getc(COM1_BASE, &r)) {
			putChar(r);
		}
	}
}
