/*
 * Copyright (c), mr-library Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-17     MacRsh       first version
 */

#include <mrlib.h>

static char *log_level_name[] =
	{
		"LOG-A",
		"LOG-E",
		"LOG-W",
		"LOG-I",
		"LOG-D",
	};

static mr_size_t _mr_log_output(const char *fmt, va_list args)
{
	mr_size_t rst = 0;
	char buffer[MR_LOG_BUFSZ];
	char value_buffer[12];
	char *string;
	mr_int32_t value;
	mr_uint32_t u_value, bits, number;
	mr_double_t d_value;

	while (*fmt != '\0')
	{
		if (*fmt == '%')
		{
			fmt ++;

			if (*fmt == '.')
			{
				fmt ++;
				number = (mr_uint32_t)(*fmt - '0');
				if (number > 6)
					number = 6;
				fmt ++;
			} else
				number = 6;

			/* Dispose %d,%x,%o,%u,%s,%c,%f */
			switch (*fmt)
			{
				/* DEC */
				case 'd':
				{
					value = va_arg(args, int);
					if (value < 0)
					{
						value = - value;
						buffer[rst ++] = '-';
					}

					bits = 0;
					do
					{
						value_buffer[bits ++] = '0' + value % 10;
						value /= 10;
					} while (value);

					while (bits)
					{
						buffer[rst ++] = value_buffer[-- bits];
					}
					break;
				}

					/* HEX */
				case 'x':
				case 'X':
				{
					u_value = va_arg(args, unsigned int);

					bits = 0;
					do
					{
						value_buffer[bits] = '0' + u_value % 16;

						if (value_buffer[bits] > '9')
							value_buffer[bits] = 'A' + (value_buffer[bits] - '9' - 1);

						u_value /= 16;
						bits ++;
					} while (u_value);

					while (bits)
					{
						buffer[rst ++] = value_buffer[-- bits];
					}
					break;
				}

					/* OCT */
				case 'o':
				{
					u_value = va_arg(args, unsigned int);

					bits = 0;
					do
					{
						value_buffer[bits] = '0' + u_value % 8;

						if (value_buffer[bits] > '9')
							value_buffer[bits] = 'A' + (value_buffer[bits] - '9' - 1);

						u_value /= 8;
						bits ++;
					} while (u_value);

					while (bits)
					{
						buffer[rst ++] = value_buffer[-- bits];
					}
					break;
				}

					/* Unsigned DEC */
				case 'u':
				{
					u_value = va_arg(args, int);

					bits = 0;
					do
					{
						value_buffer[bits ++] = '0' + u_value % 10;
						u_value /= 10;
					} while (u_value);

					while (bits)
					{
						buffer[rst ++] = value_buffer[-- bits];
					}
					break;
				}

				case 'c':
				{
					buffer[rst ++] = va_arg(args, int);
					break;
				}

				case 's':
				{
					string = va_arg(args, char *);

					while (*string != '\0')
					{
						buffer[rst ++] = *string;
						string ++;
					}
					break;
				}

				case 'f':
				{
					d_value = va_arg(args, double);
					if (d_value < 0)
					{
						d_value = - d_value;
						buffer[rst ++] = '-';
					}

					value = (mr_int32_t)d_value;
					d_value -= value;

					bits = 0;
					do
					{
						value_buffer[bits ++] = '0' + value % 10;
						value /= 10;
					} while (value);

					while (bits)
					{
						buffer[rst ++] = value_buffer[-- bits];
					}

					buffer[rst ++] = '.';
					value = (int)((d_value * 1000000.0f) + 0.5f);

					bits = 0;
					do
					{
						value_buffer[bits ++] = '0' + value % 10;
						value /= 10;
					} while (value);

					while (number)
					{
						buffer[rst ++] = value_buffer[-- bits];
						number --;
					}
					break;
				}
			}

			fmt ++;
		} else
		{
			buffer[rst ++] = *fmt;
			fmt ++;
		}
	}

	buffer[rst] = '\0';
	printf("%s", buffer);

	return rst;
}

void mr_log_output(mr_base_t level, const char *tag, const char *format, ...)
{
	va_list args;

#if (MR_LOG_OUTPUT == MR_CONF_DISABLE)
	return;
#endif

	if (level > MR_LOG_LEVEL || level < 0)
		return;

	va_start(args, format);

	mr_printf("[%s/%s]: ", log_level_name[level], tag);
	_mr_log_output(format, args);

	va_end(args);
}

void mr_assert_handler(void)
{
	while (1)
	{

	}
}



