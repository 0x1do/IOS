#include "printk.h"

void putChar(char ch)
{
	extern struct flanterm_context *ft_ctx;
	flanterm_write(ft_ctx, &ch, 1);
}

void putS(char *str)
{
	while (*str != '\0')
		putChar(*str++);
}

void longEvaluation(va_list *args, const char *str)
{
	char buf[65];
	str++;
	switch (*str) {
	case UNSIGNED_DECIMAL: {
		putS(ulongToStr(va_arg(*args, uint64_t), DECIMAL, buf));
		break;
	}
	case SIGNED_DECIMAL: {
		putS(ulongToStr(va_arg(*args, uint64_t), DECIMAL, buf));
		break;
	}
	case UNSIGNED_HEX: {
		putS(ulongToStr(va_arg(*args, uint64_t), HEX, buf));
		break;
	}
	default: {
		break;
	}
	}
}

void formatEvaluation(va_list *args, const char *str)
{
	char return_buffer[33];
	switch ((enum FormatSpecifiers) * str) {
	case SIGNED_DECIMAL:
		putS(intToStr(va_arg(*args, int), DECIMAL, return_buffer));
		break;
	case UNSIGNED_DECIMAL:
		putS(uintToStr(va_arg(*args, uint32_t), DECIMAL, return_buffer));
		break;
	case UNSIGNED_OCTAL:
		putS(uintToStr(va_arg(*args, uint32_t), OCTAL, return_buffer));
		break;
	case UNSIGNED_HEX:
		putS(uintToStr(va_arg(*args, uint32_t), HEX, return_buffer));
		break;
	case CHARACTER:
		putChar((char)va_arg(*args, int));
		break;
	case STRING:
		putS(va_arg(*args, char *));
		break;
	case POINTER_ADDRESS:
		printk("0x%lx", (uint64_t)va_arg(*args, void *));
		break;
	case MODULO:
		putChar('%');
		break;
	default:
		str--;
		break;
	default:
		str--;
		break;
	}
}

void printk(const char *fmt, ...)
{
	/*
	 *	TODO: implement va_list/va_arg/va_start
	 */
	va_list args;
	va_start(args, fmt);

	while (*fmt != '\0') {
		switch (*fmt) {
		case '%':
			fmt++;
			switch (*fmt) {
			case '\0':
				return;
			case 'l':
				longEvaluation(&args, fmt);
				fmt++;
				break;
			default:
				formatEvaluation(&args, fmt);
				break;
			}
			fmt++;
			break;
		default:
			putChar(*fmt);
			fmt++;
		}
	}

	va_end(args);
}
