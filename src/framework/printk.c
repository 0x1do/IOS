#include "printk.h"

void printChar(char ch)
{
	extern struct flanterm_context *ft_ctx;
	flanterm_write(ft_ctx, &ch, 1);
}

void puts(char *str)
{
	while (*str != '\0')
		printChar(*str++);
}

void longEvaluation(va_list *args, char *str)
{
	char buf[65];
	str++;
	switch (*str) {
	case UNSIGNED_DECIMAL: {
		puts(ulongToStr(va_arg(*args, uint64_t), DECIMAL, buf));
		break;
	}
	case SIGNED_DECIMAL: {
		puts(ulongToStr(va_arg(*args, uint64_t), DECIMAL, buf));
		break;
	}
	case UNSIGNED_HEX: {
		puts(ulongToStr(va_arg(*args, uint64_t), HEX, buf));
		break;
	}
	default: {
		break;
	}
	}
}

void formatEvaluation(va_list *args, char *str)
{
	char return_buffer[33];
	switch ((enum FormatSpecifiers) * str) {
	case SIGNED_DECIMAL:
		puts(intToStr(va_arg(*args, int), DECIMAL, return_buffer));
		break;
	case UNSIGNED_DECIMAL:
		puts(uintToStr(va_arg(*args, uint32_t), DECIMAL, return_buffer));
		break;
	case UNSIGNED_OCTAL:
		puts(uintToStr(va_arg(*args, uint32_t), OCTAL, return_buffer));
		break;
	case UNSIGNED_HEX:
		puts(uintToStr(va_arg(*args, uint32_t), HEX, return_buffer));
		break;
	case CHARACTER:
		printChar((char)va_arg(*args, int));
		break;
	case STRING:
		puts(va_arg(*args, char *));
		break;
	case POINTER_ADDRESS:
		printk("0x%lx", (uint64_t)va_arg(*args, void *));
		break;
	case MODULO:
		printChar('%');
		break;
	default:
		str--;
		break;
	}
}

void printk(char *fmt, ...)
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
			printChar(*fmt);
			fmt++;
		}
	}

	va_end(args);
}
