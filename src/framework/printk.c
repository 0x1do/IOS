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
	switch (*(str + 1)) {
	case UNSIGNED_DECIMAL:
		puts(ulongToStr(va_arg(*args, unsigned long), DECIMAL, buf));
		str++;
		break;
	case SIGNED_DECIMAL:
		puts(ulongToStr(va_arg(*args, unsigned long), DECIMAL, buf));
		str++;
		break;
	case UNSIGNED_HEX:
		puts(ulongToStr(va_arg(*args, unsigned long), HEX, buf));
		str++;
		break;
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
		puts(uintToStr(va_arg(*args, unsigned int), DECIMAL, return_buffer));
		break;
	case UNSIGNED_OCTAL:
		puts(uintToStr(va_arg(*args, unsigned int), OCTAL, return_buffer));
		break;
	case UNSIGNED_HEX:
		puts(uintToStr(va_arg(*args, unsigned int), HEX, return_buffer));
		break;
	case CHARACTER:
		printChar((char)va_arg(*args, int));
		break;
	case STRING:
		puts(va_arg(*args, char *));
		break;
	case LONG:
		longEvaluation(args, str);
		break;
	case POINTER_ADDRESS:
		printk("0x%lx", (unsigned long)va_arg(*args, void *));
		break;
	case MODULO:
		printChar('%');
		break;
	}
}

void printk(char *str, ...)
{
	/*
	 *	todo: implement va_list/va_arg/va_start
	 */
	va_list args;
	va_start(args, str);

	while (*str != '\0') {
		switch (*str) {
		case ('%'):
			str++;
			if (*str == '\0')
				continue;
			formatEvaluation(&args, str);
			str++;
			break;
		default:
			printChar(*str);
			str++;
		}
	}

	va_end(args);
}
