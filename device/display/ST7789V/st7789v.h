/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-16     MacRsh       first version
 */

#ifndef _ST7789V_H_
#define _ST7789V_H_

#include "st7789v_cfg.h"

#if (ST7789V_USING_DISPLAY_FONT == __CONFIG_ENABLE)
#include "../display_font.h"
#endif /* end of ST7789V_USING_DISPLAY_FONT */

//<<< RGB565 base color >>>
#if !defined(RGB565_WHITE)
#define RGB565_WHITE          0xFFFF
#define RGB565_BLACK          0x0000
#define RGB565_BLUE           0x001F
#define RGB565_RED            0xF800
#define RGB565_YELLOW         0xFFE0
#define RGB565_GREEN          0x07E0
#endif /* end of RGB565_WHITE */

//<<< base data type(please check that there is no conflict then change data type to typedef) >>>
#ifndef _DRIVER_DEF_H_
#define mr_uint8_t            unsigned char
#define mr_uint16_t           unsigned short
#define mr_uint32_t           unsigned int
#endif

void mr_device_st7789v_init(void);
void mr_device_st7789v_clear(void);
void mr_device_st7789v_full(mr_uint16_t color);
void mr_device_st7789v_set_background_color(mr_uint16_t color);
void mr_device_st7789v_show_region(mr_uint16_t **buffer, mr_uint16_t width, mr_uint16_t height, mr_uint16_t pos_x, mr_uint16_t pos_y);

#if (ST7789V_USING_DISPLAY_BUF == __CONFIG_ENABLE)
void mr_device_st7789v_refresh(void);
extern mr_uint16_t st7789v_buffer[ST7789V_SCR_HEIGHT][ST7789V_SCR_WIDTH];
#endif /* end of ST7789V_USING_DISPLAY_BUF */

#if (USING_ASCII_FONT_6x8 == __CONFIG_ENABLE)
void mr_device_st7789v_show_char_6x8(char data, mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y);
void mr_device_st7789v_show_string_6x8(char *string,mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y);
#endif /* end of USING_ASCII_FONT_6x8 */

#if (USING_ASCII_FONT_8x16 == __CONFIG_ENABLE)
void mr_device_st7789v_show_char_8x16(char data, mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y);
void mr_device_st7789v_show_string_8x16(char *string,mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y);
#endif  /* end of USING_8x16_FONT */

#endif /* end of _ST7789V_H_ */
