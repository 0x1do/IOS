#include "keyboard.h"
#include "printk.h"

uint8_t inb(uint16_t port)
{
	uint8_t result;
	__asm__ volatile("inb %1, %0" : "=a"(result) : "d"(port));
	return result;
}

uint8_t readKeyData()
{
	while ((inb(KEYBOARD_STATUS_PORT) & 0x01) == 0) {
		/* until data is available */
	}
	return inb(KEYBOARD_DATA_PORT);
}

static int shift_pressed = 0;

static char codeToChar(uint8_t scan_code)
{
	static uint8_t scan_code_map[] = {
		0,	 0,	   '1',	 '2', '3',	'4', '5', '6', '7', '8', '9', '0', '-',
		'=', 0,	   0,	 'q', 'w',	'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
		'[', ']',  '\n', 0,	  'a',	's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
		';', '\'', '`',	 0,	  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
		'.', '/',  0,	 '*', 0,	' ', 0,	  0,   0,	0,	 0,	  0,   0,
		0,	 '-',  0,	 '+', 0,	0,	 0,	  0,   0,	0,	 0,	  0,   0,
		0,	 0,	   0,	 0,	  0,	0,	 0,	  0,   0,	0,	 0,	  0,   0,
		0,	 0,	   0,	 0,	  0,	0,	 0,	  0,   0,	0,	 0,	  0,   0
	};

	/*
	 *TODO: make the shift and uppercase stuff to work
	 */
	if (scan_code >= 0x02 && scan_code <= 0x39) { /* If shift is pressed*/

		if (shift_pressed) {
			switch (scan_code) {
			case 0x02:
				return '!';
			case 0x03:
				return '@';
			case 0x04:
				return '#';
			case 0x05:
				return '$';
			case 0x06:
				return '%';
			case 0x07:
				return '^';
			case 0x08:
				return '&';
			case 0x09:
				return '*';
			case 0x0A:
				return '(';
			case 0x0B:
				return ')';
			case 0x0C:
				return '_';
			case 0x0D:
				return '+';
			case 0x10:
				return 'Q';
			case 0x11:
				return 'W';
			case 0x12:
				return 'E';
			case 0x13:
				return 'R';
			case 0x14:
				return 'T';
			case 0x15:
				return 'Y';
			case 0x16:
				return 'U';
			case 0x17:
				return 'I';
			case 0x18:
				return 'O';
			case 0x19:
				return 'P';
			case 0x1E:
				return 'A';
			case 0x1F:
				return 'S';
			case 0x20:
				return 'D';
			case 0x21:
				return 'F';
			case 0x22:
				return 'G';
			case 0x23:
				return 'H';
			case 0x24:
				return 'J';
			case 0x25:
				return 'K';
			case 0x26:
				return 'L';
			case 0x2C:
				return 'Z';
			case 0x2D:
				return 'X';
			case 0x2E:
				return 'C';
			case 0x2F:
				return 'V';
			case 0x30:
				return 'B';
			case 0x31:
				return 'N';
			case 0x32:
				return 'M';
			default:
				return scan_code_map[scan_code];
			}
		}
		return scan_code_map[scan_code];
	}
	return 0;
}

void printScanCode(uint8_t scan_code)
{
	printk("=====KEY SCAN CODE=======\n");
	printk("           %c        \n", codeToChar(scan_code));
	printk("=========================\n");
}

void keyboardPoll()
{
	uint8_t key;
	while (1) {
		key = readKeyData();
	}
}

char getChar()
{
	uint8_t code;
	while (1) {
		code = readKeyData();
		if (code != 0) {
			char key = codeToChar(code);
			if (key != 0) {
				printChar(key);
				return key;
			}
		}
	}
}

void scanf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char *str;
	char key;
	int scanned_count = 0;

	while (*fmt) {
		if (*fmt == '%') {
			fmt++;

			if (*fmt == 's') {
				str = va_arg(args, char *);
				char *arg = str;

				while (1) {
					key = getChar();

					if (key == '\n' || key == '\r') {
						*arg = '\0';
						break;
					}

					if (key == ' ') {
						if (arg != str) {
							*arg = '\0';
							arg++;
						}
						continue;
					}

					*arg = key;
					arg++;
				}
				scanned_count++;
			}
		} else {
			printChar((char)*fmt);
		}
		fmt++;
	}

	va_end(args);
}
