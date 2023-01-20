/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-20     MacRsh       first version
 */

#include "mr_printf.h"

/** user specification :
 *  please set the configuration file first !!!
 *
 *  if you want to use the mr_printf library, adapt the following api:
 *  (must to adapt) mr_putc : output char through the hardware layer, function format: void mr_putc(char data);
 *
 *  then, you can use the following api:
 *  (can use) mr_printf : such as mr_printf in the libc
 *
 *  maybe you will run out of memory, you can crop mr_printf library
 *
 */

#if (USING_MR_PRINTF == __CONFIG_ENABLE)

MR_WEAK void mr_putc(char data)
{

}

int mr_printf(char *fmt, ...)
{
  va_list ap;
  char putc_buf[20];
  unsigned int u_val;
  int val, bits, flag;
  double f_val;
  char *str;
  int res = 0;

  /* move ap to fmt + sizeof(fmt) */
  va_start(ap,fmt);

  while (*fmt != '\0')
  {
    if(*fmt == '%')
    {
      ++ fmt;

      /* dispose %.x */
      if(*fmt == '.')
      {
        ++ fmt;
        flag = (int)(*fmt - '0');
        ++ fmt;
      }
      else
      {
        flag = 187; // N(46) + U(53) + L(44) + L(44) = NULL(187)
      }

      /* dispose %d,%x,%o,%u,%s,%c,%f */
      switch (*fmt)
      {
#if (USING_MR_PRINTF_DEC == __CONFIG_ENABLE)
        /* mr_printf signed int to DEC */
        case 'd':
          /* get value */
          val = va_arg(ap,int);
          if(val < 0)
          {
            val = - val;
            mr_putc('-');
            ++ res;
          }

          /* get value bits */
          bits = 0;
          while(val)
          {
            putc_buf[bits] = '0' + val % 10;
            val /= 10;
            ++ bits;
          }
          res += bits;

          /* put value bits */
          while (bits)
          {
            -- bits;
            mr_putc(putc_buf[bits]);
          }
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_DEC */

#if (USING_MR_PRINTF_HEX == __CONFIG_ENABLE)
        /* mr_printf unsigned int to HEX */
        case 'x':
          /* get value */
          u_val = va_arg(ap,unsigned int);

          /* get value bits */
          bits = 0;
          while(u_val)
          {
            putc_buf[bits] = '0' + u_val % 16;

            if(putc_buf[bits] > '9')
              putc_buf[bits] = 'A' + (putc_buf[bits] - '9' - 1);

            u_val /= 16;
            ++ bits;
          }
          res += bits;

          /* put value bits */
          while (bits)
          {
            -- bits;
            mr_putc(putc_buf[bits]);
          }
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_HEX */

#if (USING_MR_PRINTF_OCT == __CONFIG_ENABLE)
        /* mr_printf unsigned int to OCT */
        case 'o':
          /* get value */
          u_val = va_arg(ap,unsigned int);

          /* get value bits */
          bits = 0;
          while(u_val)
          {
            putc_buf[bits] = '0' + u_val % 8;
            u_val /= 8;
            ++ bits;
          }
          res += bits;

          /* put value bits */
          while (bits)
          {
            -- bits;
            mr_putc(putc_buf[bits]);
          }
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_OCT */

#if (USING_MR_PRINTF_UNSIGNED == __CONFIG_ENABLE)
        /* mr_printf unsigned int to DEC */
        case 'u':
          /* get value */
          u_val = va_arg(ap,unsigned int);

          /* get value bits */
          bits = 0;
          while(u_val)
          {
            putc_buf[bits] = '0' + u_val % 10;
            u_val /= 10;
            ++ bits;
          }
          res += bits;

          /* put value bits */
          while (bits)
          {
            -- bits;
            mr_putc(putc_buf[bits]);
          }
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_UNSIGNED */

#if (USING_MR_PRINTF_CHAR == __CONFIG_ENABLE)
        /* mr_printf char */
        case 'c':
          mr_putc(va_arg(ap,int));
          ++ res;
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_CHAR */

#if (USING_MR_PRINTF_STRING == __CONFIG_ENABLE)
        /* mr_printf string */
        case 's':
          str = va_arg(ap,char *);
          while (*str != '\0')
          {
            mr_putc(*str);
            ++ str;
            ++ res;
          }
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_STRING */

#if (USING_MR_PRINTF_FLOAT == __CONFIG_ENABLE)
          /* mr_printf float */
        case 'f':
          /* get value */
          f_val = va_arg(ap,double);
          if(f_val < 0)
          {
            f_val = - f_val;
            mr_putc('-');
            ++ res;
          }

          /* separation int and float */
          val = (int)f_val;
          f_val -= (double)val;

          /* get int value bits */
          bits = 0;
          if(val == 0)
          {
            mr_putc('0');
            ++ res;
          }
          while (val)
          {
            putc_buf[bits] = '0' + val % 10;
            val /= 10;
            ++ bits;
          }
          res += bits;

          /* put int value bits */
          while (bits)
          {
            --bits;
            mr_putc(putc_buf[bits]);
          }

          /* dispose float */
          if(flag != 0)
          {
            mr_putc('.');
            ++ res;
          }
          if(flag > 6)
            flag = 6;
          val = (int)((f_val * 1000000.0f) + 0.5f);

          /* get float value bits */
          bits = 0;
          while (bits < 6)
          {
            putc_buf[bits] = '0' + val % 10;
            val /= 10;
            ++ bits;
          }
          res += flag;

          /* put int value bits */
          while (flag)
          {
            --flag;
            -- bits;
            mr_putc(putc_buf[bits]);
          }
          ++ fmt;
          continue;
#endif /* end of USING_MR_PRINTF_FLOAT */

        default:
          -- fmt;
          mr_putc(*fmt);
          ++ res;
          ++ fmt;
          continue;
      }
    }
    else
    {
      mr_putc(*fmt);
      ++ res;
      ++ fmt;
    }
  }

  /* set ap = null */
  va_end(ap);

  return res;
}

#endif /* end of USING_MR_PRINTF */