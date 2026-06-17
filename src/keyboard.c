#include "keyboard.h"
#include "msg.h"
#include "printk.h"

static int g_shift = 0;

uint8_t inb(uint16_t port)
{
	uint8_t result;
	__asm__ volatile("inb %1, %0" : "=a"(result) : "d"(port));
	return result;
}

void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile("outb %0, %1" ::"a"(val), "d"(port));
}

uint8_t readKeyData()
{
	while ((inb(KEYBOARD_STATUS_PORT) & 0x01) == 0) {
		msg_poll();
	}
	return inb(KEYBOARD_DATA_PORT);
}

void clearKeyboardBuffer()
{
	while (inb(KEYBOARD_STATUS_PORT) & 0x01) {
		inb(KEYBOARD_DATA_PORT);
	}
}

char codeToChar(uint8_t scan_code)
{
	static uint8_t scan_code_map[] = {
		0,	 0,	   '1',	 '2', '3',	'4', '5', '6', '7', '8', '9', '0', '-',
		'=', '\b', 0,	 'q', 'w',	'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
		'[', ']',  '\n', 0,	  'a',	's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
		';', '\'', '`',	 0,	  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
		'.', '/',  0,	 '*', 0,	' ', 0,	  0,   0,	0,	 0,	  0,   0,
		0,	 '-',  0,	 '+', 0,	0,	 0,	  0,   0,	0,	 0,	  0,   0,
		0,	 0,	   0,	 0,	  0,	0,	 0,	  0,   0,	0,	 0,	  0,   0,
		0,	 0,	   0,	 0,	  0,	0,	 0,	  0,   0,	0,	 0,	  0,   0
	};

	if (scan_code >= 0x02 && scan_code <= 0x39) {
		if (g_shift) {
			switch (scan_code) {
			case 0x02: return '!';
			case 0x03: return '@';
			case 0x04: return '#';
			case 0x05: return '$';
			case 0x06: return '%';
			case 0x07: return '^';
			case 0x08: return '&';
			case 0x09: return '*';
			case 0x0A: return '(';
			case 0x0B: return ')';
			case 0x0C: return '_';
			case 0x0D: return '+';
			case 0x10: return 'Q';
			case 0x11: return 'W';
			case 0x12: return 'E';
			case 0x13: return 'R';
			case 0x14: return 'T';
			case 0x15: return 'Y';
			case 0x16: return 'U';
			case 0x17: return 'I';
			case 0x18: return 'O';
			case 0x19: return 'P';
			case 0x1A: return '{';
			case 0x1B: return '}';
			case 0x1E: return 'A';
			case 0x1F: return 'S';
			case 0x20: return 'D';
			case 0x21: return 'F';
			case 0x22: return 'G';
			case 0x23: return 'H';
			case 0x24: return 'J';
			case 0x25: return 'K';
			case 0x26: return 'L';
			case 0x27: return ':';
			case 0x28: return '"';
			case 0x29: return '~';
			case 0x2B: return '|';
			case 0x2C: return 'Z';
			case 0x2D: return 'X';
			case 0x2E: return 'C';
			case 0x2F: return 'V';
			case 0x30: return 'B';
			case 0x31: return 'N';
			case 0x32: return 'M';
			case 0x33: return '<';
			case 0x34: return '>';
			case 0x35: return '?';
			default: return scan_code_map[scan_code];
			}
		}
		return scan_code_map[scan_code];
	}
	return 0;
}

void printScanCode(uint8_t scan_code)
{
	printk("\n=====KEY SCAN CODE=======\n");
	printk("          0x%x        \n", scan_code);
	printk("           %c        \n", codeToChar(scan_code));
	printk("=========================\n");
}

void keyboardPoll()
{
	char ch;
	while (1) {
		ch = readKeyData();
		printScanCode(ch);
	}
}

char getChar()
{
	uint8_t code;
	while (1) {
		code = readKeyData();
		if (code == 0x2A || code == 0x36) {
			g_shift = 1;
			continue;
		}
		if (code == 0xAA || code == 0xB6) {
			g_shift = 0;
			continue;
		}
		if (code & 0x80)
			continue;
		if (code != 0) {
			char ch = codeToChar(code);
			if (ch == '\b')
				return ch;

			if (ch != 0) {
				putChar(ch);
				return ch;
			}
		}
	}
}

int getCharEx(void)
{
	uint8_t code;
	while (1) {
		code = readKeyData();
		if (code == 0xE0) {
			code = readKeyData();
			switch (code) {
			case 0x48: return KEY_UP;
			case 0x50: return KEY_DOWN;
			case 0x4B: return KEY_LEFT;
			case 0x4D: return KEY_RIGHT;
			case 0x47: return KEY_HOME;
			case 0x4F: return KEY_END;
			case 0x53: return KEY_DEL;
			}
			continue;
		}
		if (code == 0x2A || code == 0x36) {
			g_shift = 1;
			continue;
		}
		if (code == 0xAA || code == 0xB6) {
			g_shift = 0;
			continue;
		}
		if (code & 0x80)
			continue;
		if (code != 0) {
			char ch = codeToChar(code);
			if (ch != 0)
				return ch;
		}
	}
}

typedef int (*getCharFunc)(void *context);

int getCharWrapper(void *context)
{
	(void)context; // Unused parameter
	return (int)getChar();
}

int getCharFromString(void *context)
{
	char **p = (char **)context;
	if (**p == '\0')
		return -1;
	return *(*p)++;
}

int scankCore(getCharFunc getc, void *context, const char *fmt, va_list args)
{
	int ch;
	int scanned_count = 0;
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;

			if (*fmt == 's') {
				char *str = va_arg(args, char *);
				char *arg = str;

				while (1) {
					ch = getc(context);

					if (ch == -1 || ch == '\n' || ch == '\r') {
						*arg = '\0';
						break;
					}

					if (ch == ' ') {
						if (arg != str) {
							*arg = '\0';
							arg++;
						}
						continue;
					}
					if (ch == '\b') {
						*arg = '\0'; /* without it, asdf<\b> will return asdf */
						if (arg != str)
							arg--;
						continue;
					}

					*arg = ch;
					arg++;
				}
				scanned_count++;
			} else if (*fmt == 'd') {
				int *out = va_arg(args, int *);
				int value = 0;
				int sign = 1;
				int digits = 0;

				ch = getc(context);
				if (ch == '-') {
					sign = -1;
					ch = getc(context);
				}
				while (ch >= '0' && ch <= '9') {
					value = value * 10 + (ch - '0');
					digits++;
					ch = getc(context);
				}
				if (digits == 0)
					break;
				*out = sign * value;
				scanned_count++;
			}
		} else {
			ch = getc(context);
			if (ch != *fmt)
				break;
		}
		fmt++;
	}

	return scanned_count;
}

int scank(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int count = scankCore(getCharWrapper, NULL, fmt, args);
	va_end(args);
	return count;
}

int sscank(const char *input, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *p = (char *)input;
	int count = scankCore(getCharFromString, &p, fmt, args);
	va_end(args);
	return count;
}

int fgetsk_core(char *buf, int size, getCharFunc getc, void *context)
{
	if (size <= 0)
		return 0;

	int i = 0;
	int ch;

	while (i < size - 1) { /* excluding null terminator */
		ch = getc(context);
		if (ch == -1)
			break;
		if (ch == '\n') {
			putChar('\n');
			break;
		}
		if (ch == '\b') {
			if (i > 0) {
				i--;
				putChar('\b');
				putChar(' ');
				putChar('\b');
			}
			continue;
		}
		buf[i++] = (char)ch;
	}
	buf[i] = '\0';
	return i;
}

int fgetsk(char *buf, int size)
{
	return fgetsk_core(buf, size, getCharWrapper, NULL);
}
