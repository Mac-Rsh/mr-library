/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-16     MacRsh       first version
 */

#include "st7789v.h"

/** user specification :
 *  please set the configuration file first !!!
 *
 *  the st7789v screen library rely on spi-bus driver
 *  if you want to use the st7789v screen library, adapt the following api:
 *  (must to adapt) SET_DELAY_MS : set the delay in milliseconds
 *  (must to adapt) SET_RST_LOW : set the reset line to low level
 *  (must to adapt) SET_RST_HIGH : set the reset line to high level
 *  (must to adapt) SET_DC_LOW : set the data/command line to low level
 *  (must to adapt) SET_DC_HIGH : set the data/command line to low level
 *  (must to adapt) mr_device_st7789v_hw_init : initialize (reset - data/command - choose) gpio and spi-bus
 *  (must to adapt) mr_device_st7789v_hw_write_8bit : spi-bus write 8bit data to screen
 *  (optional to adapt) SET_CS_LOW : set the choose line to low level
 *  (optional to adapt) SET_CS_HIGH : set the choose line to high level
 *
 *  then, you can use the following api:
 *  (can use) mr_device_st7789v_init : initialize the st7789v screen
 *  (can use) mr_device_st7789v_clear : clear the screen to background color
 *  (can use) mr_device_st7789v_full : full the screen to color
 *  (can use) mr_device_st7789v_set_background_color : set the background color(the settings take effect only after being cleared)
 *  (can use) mr_device_st7789v_show_region : show image to selectable region
 *  (can use) mr_device_st7789v_refresh : refresh screen to buffer image
 *  (can use) mr_device_st7789v_show_char_6x8 : show 6*8 size char
 *  (can use) mr_device_st7789v_show_string_6x8 : show 6*8 size string
 *  (can use) mr_device_st7789v_show_char_8x16 : show 8*16 size char
 *  (can use) mr_device_st7789v_show_string_8x16 : show 8*16 size string
 *
 *  maybe you will run out of memory, you can crop font library
 *
 */

//<---------------------------- Hardware (user need to adapt)----------------------------------------->
//< must >
//<<< you must to adapt the following api >>>
#define SET_DELAY_MS(ms)            Delay_Ms(ms)
#define SET_RST_LOW()               GPIO_WriteBit(GPIOA,GPIO_Pin_4,0)
#define SET_RST_HIGH()              GPIO_WriteBit(GPIOA,GPIO_Pin_4,1)
#define SET_DC_LOW()                GPIO_WriteBit(GPIOA,GPIO_Pin_3,0)
#define SET_DC_HIGH()               GPIO_WriteBit(GPIOA,GPIO_Pin_3,1)
//< optional >
//<<< you optional to adapt the following api >>>
#define SET_CS_LOW()                GPIO_WriteBit(GPIOA,GPIO_Pin_2,0)
#define SET_CS_HIGH()               GPIO_WriteBit(GPIOA,GPIO_Pin_2,1)
// < must >
//<<< you must to adapt the following api >>>
static void mr_device_st7789v_hw_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA,GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
}
// < must >
//<<< you must to adapt the following api >>>
static void mr_device_st7789v_hw_write_8bit(mr_uint8_t data)
{
  //mr_device_soft_spi_bus_transmit(1,data);
}

//<---------------------------- Driver (user don't need to care)----------------------------------------->
mr_uint16_t st7789v_x_max,st7789v_y_max;
mr_uint16_t st7789v_dir;
mr_uint16_t st7789v_background_color;

#if (ST7789V_USING_DISPLAY_BUF == __CONFIG_ENABLE)
mr_uint16_t st7789v_buffer[ST7789V_SCR_HEIGHT][ST7789V_SCR_WIDTH] =
{
  [0 ... (ST7789V_SCR_HEIGHT - 1)][0 ... (ST7789V_SCR_WIDTH - 1)] = ST7789V_DISPLAY_COLOR
};
#endif /* end of ST7789V_USING_DISPLAY_BUF */

/**
 * This function will write index data to screen
 *
 * @param data the display index data
 *
 */
static void mr_device_st7789v_write_index(mr_uint8_t data)
{
#if (ST7789V_CS_AUTO_CTRL == __CONFIG_ENABLE)
  SET_CS_HIGH();
  SET_CS_LOW();
#endif

  SET_DC_LOW();
  mr_device_st7789v_hw_write_8bit(data);
  SET_DC_HIGH();

#if (ST7789V_CS_AUTO_CTRL == __CONFIG_ENABLE)
  SET_CS_HIGH();
#endif
}

/**
 * This function will write 8bit data to screen
 *
 * @param data the display data
 *
 */
static void mr_device_st7789v_write_8bit(mr_uint8_t data)
{
#if (ST7789V_CS_AUTO_CTRL == __CONFIG_ENABLE)
  SET_CS_HIGH();
  SET_CS_LOW();
#endif

  mr_device_st7789v_hw_write_8bit(data);

#if (ST7789V_CS_AUTO_CTRL == __CONFIG_ENABLE)
  SET_CS_HIGH();
#endif
}

/**
 * This function will write 16bit data to screen
 *
 * @param data the display data
 *
 */
static void mr_device_st7789v_write_16bit_data(mr_uint16_t data)
{
#if (ST7789V_CS_AUTO_CTRL == __CONFIG_ENABLE)
  SET_CS_HIGH();
  SET_CS_LOW();
#endif

  mr_device_st7789v_hw_write_8bit(data >> 8);
  mr_device_st7789v_hw_write_8bit(data & 0x00FF);

#if (ST7789V_CS_AUTO_CTRL == __CONFIG_ENABLE)
  SET_CS_HIGH();
#endif
}

/**
 * This function will set display direction(angle)
 *
 * @param angle the display angle
 *
 */
static void mr_device_st7789v_set_display_dir(mr_uint16_t angle)
{
  st7789v_dir = angle;

  if((st7789v_dir % 180) == 0)
  {
    st7789v_x_max = ST7789V_SCR_WIDTH;
    st7789v_y_max = ST7789V_SCR_HEIGHT;
  }
  else
  {
    st7789v_x_max = ST7789V_SCR_HEIGHT;
    st7789v_y_max = ST7789V_SCR_WIDTH;
  }
}

/**
 * This function will set display region to st7789v
 *
 * @param x1 the display X coord 1
 * @param y1 the display Y coord 1
 * @param x2 the display X coord 2
 * @param y2 the display Y coord 2
 *
 */
static void mr_device_st7789v_set_region(mr_uint16_t x1, mr_uint16_t y1, mr_uint16_t x2, mr_uint16_t y2)
{
  switch (st7789v_dir) {
    case 0:
      mr_device_st7789v_write_index(0x2a);
      mr_device_st7789v_write_16bit_data(x1 + 40);
      mr_device_st7789v_write_16bit_data(x2 + 40);
      mr_device_st7789v_write_index(0x2b);
      mr_device_st7789v_write_16bit_data(y1 + 53);
      mr_device_st7789v_write_16bit_data(y2 + 53);
      mr_device_st7789v_write_index(0x2c);
      break;
    case 90:
      mr_device_st7789v_write_index(0x2a);
      mr_device_st7789v_write_16bit_data(x1 + 52);
      mr_device_st7789v_write_16bit_data(x2 + 52);
      mr_device_st7789v_write_index(0x2b);
      mr_device_st7789v_write_16bit_data(y1 + 40);
      mr_device_st7789v_write_16bit_data(y2 + 40);
      mr_device_st7789v_write_index(0x2c);
      break;
    case 180:
      mr_device_st7789v_write_index(0x2a);
      mr_device_st7789v_write_16bit_data(x1 + 40);
      mr_device_st7789v_write_16bit_data(x2 + 40);
      mr_device_st7789v_write_index(0x2b);
      mr_device_st7789v_write_16bit_data(y1 + 52);
      mr_device_st7789v_write_16bit_data(y2 + 52);
      mr_device_st7789v_write_index(0x2c);
      break;
    case 270:
      mr_device_st7789v_write_index(0x2a);
      mr_device_st7789v_write_16bit_data(x1 + 53);
      mr_device_st7789v_write_16bit_data(x2 + 53);
      mr_device_st7789v_write_index(0x2b);
      mr_device_st7789v_write_16bit_data(y1 + 40);
      mr_device_st7789v_write_16bit_data(y2 + 40);
      mr_device_st7789v_write_index(0x2c);
      break;
  }
}

/**
 * This function will clear display as background color
 *
 */
void mr_device_st7789v_clear(void)
{
  mr_uint32_t i = st7789v_x_max * st7789v_y_max;

  mr_device_st7789v_set_region(0,0,st7789v_x_max - 1,st7789v_y_max - 1);
  while (i--)
  {
    mr_device_st7789v_write_16bit_data(st7789v_background_color);
  }
}

/**
 * This function will set display background color
 *
 * @param color the display color
 *
 */
void mr_device_st7789v_set_background_color(mr_uint16_t color)
{
  st7789v_background_color = color;
}

/**
 * This function will full display as set color
 *
 * @param color the display full color
 *
 */
void mr_device_st7789v_full(mr_uint16_t color)
{
  mr_uint32_t i = st7789v_x_max * st7789v_y_max;

  mr_device_st7789v_set_region(0,0,st7789v_x_max - 1,st7789v_y_max - 1);
  while (i--)
  {
    mr_device_st7789v_write_16bit_data(color);
  }
}

/**
 * This function will initialize screen
 *
 */
void mr_device_st7789v_init(void)
{
  mr_device_st7789v_hw_init();

  mr_device_st7789v_set_display_dir(ST7789V_DISPLAY_DIR);
  mr_device_st7789v_set_background_color(ST7789V_DISPLAY_COLOR);

  SET_RST_LOW();
  SET_DELAY_MS(200);
  SET_RST_HIGH();
  SET_DELAY_MS(200);

  mr_device_st7789v_write_index(0x36);
  SET_DELAY_MS(100);

  switch (st7789v_dir) {
    case 0:
      mr_device_st7789v_write_8bit(0x70);
      break;
    case 90:
      mr_device_st7789v_write_8bit(0x00);
      break;
    case 180:
      mr_device_st7789v_write_8bit(0xA0);
      break;
    case 270:
      mr_device_st7789v_write_8bit(0xC0);
      break;
  }

  mr_device_st7789v_write_index(0x3A);
  mr_device_st7789v_write_8bit(0x05);

  mr_device_st7789v_write_index(0xB2);
  mr_device_st7789v_write_8bit(0x0C);
  mr_device_st7789v_write_8bit(0x0C);
  mr_device_st7789v_write_8bit(0x00);
  mr_device_st7789v_write_8bit(0x33);
  mr_device_st7789v_write_8bit(0x33);

  mr_device_st7789v_write_index(0xB7);
  mr_device_st7789v_write_8bit(0x35);

  mr_device_st7789v_write_index(0xBB);
  mr_device_st7789v_write_8bit(0x37);

  mr_device_st7789v_write_index(0xC0);
  mr_device_st7789v_write_8bit(0x2C);

  mr_device_st7789v_write_index(0xC2);
  mr_device_st7789v_write_8bit(0x01);

  mr_device_st7789v_write_index(0xC3);
  mr_device_st7789v_write_8bit(0x12);

  mr_device_st7789v_write_index(0xC4);
  mr_device_st7789v_write_8bit(0x20);

  mr_device_st7789v_write_index(0xC6);
  mr_device_st7789v_write_8bit(0x0F);

  mr_device_st7789v_write_index(0xD0);
  mr_device_st7789v_write_8bit(0xA4);
  mr_device_st7789v_write_8bit(0xA1);

  mr_device_st7789v_write_index(0xE0);
  mr_device_st7789v_write_8bit(0xD0);
  mr_device_st7789v_write_8bit(0x04);
  mr_device_st7789v_write_8bit(0x0D);
  mr_device_st7789v_write_8bit(0x11);
  mr_device_st7789v_write_8bit(0x13);
  mr_device_st7789v_write_8bit(0x2B);
  mr_device_st7789v_write_8bit(0x3F);
  mr_device_st7789v_write_8bit(0x54);
  mr_device_st7789v_write_8bit(0x4C);
  mr_device_st7789v_write_8bit(0x18);
  mr_device_st7789v_write_8bit(0x0D);
  mr_device_st7789v_write_8bit(0x0B);
  mr_device_st7789v_write_8bit(0x1F);
  mr_device_st7789v_write_8bit(0x23);

  mr_device_st7789v_write_index(0xE1);
  mr_device_st7789v_write_8bit(0xD0);
  mr_device_st7789v_write_8bit(0x04);
  mr_device_st7789v_write_8bit(0x0C);
  mr_device_st7789v_write_8bit(0x11);
  mr_device_st7789v_write_8bit(0x13);
  mr_device_st7789v_write_8bit(0x2C);
  mr_device_st7789v_write_8bit(0x3F);
  mr_device_st7789v_write_8bit(0x44);
  mr_device_st7789v_write_8bit(0x51);
  mr_device_st7789v_write_8bit(0x2F);
  mr_device_st7789v_write_8bit(0x1F);
  mr_device_st7789v_write_8bit(0x1F);
  mr_device_st7789v_write_8bit(0x20);
  mr_device_st7789v_write_8bit(0x23);

  mr_device_st7789v_write_index(0x21);

  mr_device_st7789v_write_index(0x11);
  SET_DELAY_MS(120);

  mr_device_st7789v_write_index(0x29);

  mr_device_st7789v_clear();
}

/**
 * This function will show image to selectable region
 *
 * @param buffer the display buffer
 * @param width the display buffer width
 * @param height the display buffer height
 * @param pos_x the display position X on the screen
 * @param pos_y the display position Y on the screen
 *
 */
void mr_device_st7789v_show_region(mr_uint16_t **buffer, mr_uint16_t width, mr_uint16_t height, mr_uint16_t pos_x, mr_uint16_t pos_y)
{
  mr_uint32_t i_width, i_height;

  mr_device_st7789v_set_region(pos_x,pos_y,(pos_x + width) - 1,(pos_y + height) - 1);

  for (i_height = 0; i_height < height; i_height ++)
  {
    for(i_width = 0; i_width < width; i_width ++)
    {
      mr_device_st7789v_write_16bit_data(*((mr_uint16_t *)buffer + i_height * width + i_width));
    }
  }
}

#if (ST7789V_USING_DISPLAY_BUF == __CONFIG_ENABLE)
/**
 * This function will refresh display
 *
 */
void mr_device_st7789v_refresh(void)
{
  mr_uint32_t width, height;

  mr_device_st7789v_set_region(0,0,st7789v_x_max - 1,st7789v_y_max - 1);

  for (height = 0; height < st7789v_y_max; height ++)
  {
    for(width = 0; width < st7789v_x_max; width ++)
    {
      mr_device_st7789v_write_16bit_data(st7789v_buffer[height][width]);
    }
  }
}
#endif /* end of ST7789V_USING_DISPLAY_BUF */

#if (ST7789V_USING_DISPLAY_FONT == __CONFIG_ENABLE)

#if (USING_ASCII_FONT_6x8 == __CONFIG_ENABLE)
/**
 * This function will show 6*8 size char
 *
 * @param data char data
 * @param color char color
 * @param pos_x the display position X on the screen
 * @param pos_y the display position Y on the screen
 */
void mr_device_st7789v_show_char_6x8(char data, mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y)
{
  mr_uint32_t width, height;

  for (height = 0; height < 6; height ++)
  {
    mr_device_st7789v_set_region(pos_x + height,pos_y,pos_x + height,pos_y + 8);

    uint8_t font = ascii_font_6x8[data - 32][height];
    for(width = 0; width < 8; width ++)
    {
      if(font & 0x01)
        mr_device_st7789v_write_16bit_data(color);
      else
        mr_device_st7789v_write_16bit_data(st7789v_background_color);

      font >>= 1;
    }
  }
}

/**
 * This function will show 6*8 size string
 *
 * @param string string data
 * @param color char color
 * @param pos_x the display position X on the screen
 * @param pos_y the display position Y on the screen
 */
void mr_device_st7789v_show_string_6x8(char *string,mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y)
{
  while(*string != '\0')
  {
    mr_device_st7789v_show_char_6x8(*string,color,pos_x,pos_y);

    ++ string;
    pos_x += 6;
  }
}
#endif /* end of USING_ASCII_FONT_6x8 */

#if (USING_ASCII_FONT_8x16 == __CONFIG_ENABLE)
/**
 * This function will show 8*16 size char
 *
 * @param data char data
 * @param color char color
 * @param pos_x the display position X on the screen
 * @param pos_y the display position Y on the screen
 */
void mr_device_st7789v_show_char_8x16(char data, mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y)
{
  mr_uint32_t width, height;

  for (height = 0; height < 8; height ++)
  {
    mr_device_st7789v_set_region(pos_x + height,pos_y,pos_x + height,pos_y + 15);

    uint8_t font = ascii_font_8x16[data - 32][height];
    for(width = 0; width < 8; width ++)
    {
      if(font & 0x01)
        mr_device_st7789v_write_16bit_data(color);
      else
        mr_device_st7789v_write_16bit_data(st7789v_background_color);

      font >>= 1;
    }

    font = ascii_font_8x16[data - 32][height + 8];
    for(width = 0; width < 8; width ++)
    {
      if(font & 0x01)
        mr_device_st7789v_write_16bit_data(color);
      else
        mr_device_st7789v_write_16bit_data(st7789v_background_color);

      font >>= 1;
    }
  }
}

/**
 * This function will show 8*16 size string
 *
 * @param string string data
 * @param color char color
 * @param pos_x the display position X on the screen
 * @param pos_y the display position Y on the screen
 */
void mr_device_st7789v_show_string_8x16(char *string,mr_uint16_t color, mr_uint16_t pos_x, mr_uint16_t pos_y)
{
  while(*string != '\0')
  {
    mr_device_st7789v_show_char_8x16(*string,color,pos_x,pos_y);

    ++ string;
    pos_x += 8;
  }
}
#endif  /* end of USING_8x16_FONT */

#endif /* end of ST7789V_USING_DISPLAY_FONT */
