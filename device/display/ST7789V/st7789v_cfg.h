/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-16     MacRsh       first version
 */

#ifndef _ST7789V_CFG_H_
#define _ST7789V_CFG_H_

/** user specification :
 *  ( __CONFIG_ENABLE ) it means enable function
 *  ( __CONFIG_DISABLE ) it means disable function
 *  ( //< must > ) it means this option need user complement
 *  ( //< const > ) it means this option unchangeable
 *  ( //< change able > ) it means this option can change, you could change parameter or annotation parameter
 *  ( //< optional > ) it means this option is optional, not essential
 *  ( //<<< XXXX >>> ) it means interpretation this define means
 */

#define __CONFIG_ENABLE               1
#define __CONFIG_DISABLE              0

//<---------------------------- Hardware -------------------------------------->
//< must >
//<<< If you not using mr_device frame, please remove the following include, replace with your chip head-file >>>
#include "device_def.h"
//< change able >
//<<< Using the screen library comes with spi-cs control >>>
#define ST7789V_CS_AUTO_CTRL          __CONFIG_ENABLE
//<<< The width of screen >>>
#define ST7789V_SCR_WIDTH             240
//<<< The height of screen >>>
#define ST7789V_SCR_HEIGHT            135

//<---------------------------- FRAME ----------------------------------------->
//< change able >
//<<< The screen displays the direction by default(angle), default: 0 >>>
#define ST7789V_DISPLAY_DIR           0
//<<< The screen display background color, default: 0xFFFF >>>
#define ST7789V_DISPLAY_COLOR         0xFFFF
//<<< Using display buffer, the entire screen will refresh at once, occupation: (ST7789V_SCR_WIDTH * ST7789V_SCR_HEIGHT) * 2byte >>>
#define ST7789V_USING_DISPLAY_BUF     __CONFIG_DISABLE
//<<< Using display font >>>
#define ST7789V_USING_DISPLAY_FONT    __CONFIG_ENABLE

#endif /* end of _ST7789V_CFG_H_ */
