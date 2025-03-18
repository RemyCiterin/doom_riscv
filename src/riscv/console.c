/*
 * console.c
 *
 * Copyright (C) 2019-2021 Sylvain Munaut
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdint.h>

#include "config.h"
#include "mini-printf.h"


static volatile uint8_t* uart_data = (uint8_t*)UART_BASE;

void
console_init(void) {}

void
console_putchar(char c)
{
	*uart_data = c;
}

char
console_getchar(void)
{
	return *uart_data;
}

int
console_getchar_nowait(void)
{
  return *uart_data;
}

void
console_puts(const char *p)
{
	char c;
	while ((c = *(p++)) != 0x00)
		*uart_data = c;
}

int console_printf(const char *fmt, ...)
{
	static char _printf_buf[128];
        va_list va;
        int l;

        va_start(va, fmt);
        l = mini_vsnprintf(_printf_buf, 128, fmt, va);
        va_end(va);

	console_puts(_printf_buf);

	return l;
}
