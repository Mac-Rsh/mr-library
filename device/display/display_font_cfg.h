/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-16     MacRsh       first version
 */

#ifndef _DISPLAY_FONT_CFG_H_
#define _DISPLAY_FONT_CFG_H_

/** user specification :
 *  ( __CONFIG_ENABLE ) it means enable function
 *  ( __CONFIG_DISABLE ) it means disable function
 *  ( //< must > ) it means this option need user complement
 *  ( //< const > ) it means this option unchangeable
 *  ( //< change able > ) it means this option can change, you could change parameter or annotation parameter
 *  ( //< optional > ) it means this option is optional, not essential
 *  ( //<<< XXXX >>> ) it means interpretation this define means
 */

#define __CONFIG_ENABLE                       1
#define __CONFIG_DISABLE                      0

//<---------------------------- FRAME ----------------------------------------->
//< change able >
//<<< Using the 6*8 size ascii font >>>
#define USING_ASCII_FONT_6x8                  __CONFIG_ENABLE
//<<< Using the 8*16 size ascii font >>>
#define USING_ASCII_FONT_8x16                 __CONFIG_ENABLE
//<<< Using the 6*8 size figure font >>>
#define USING_FIGURE_FONT_6x8                 __CONFIG_ENABLE
//<<< Using the 8*16 size figure font >>>
#define USING_FIGURE_FONT_8x16                __CONFIG_ENABLE
//<<< Using the 6*8 size capital letter font >>>
#define USING_CAPITAL_LETTER_FONT_6x8         __CONFIG_ENABLE
//<<< Using the 8*16 size capital letter font >>>
#define USING_CAPITAL_LETTER_FONT_8x16        __CONFIG_ENABLE
//<<< Using the 6*8 size small letter font >>>
#define USING_SMALL_LETTER_FONT_6x8           __CONFIG_ENABLE
//<<< Using the 8*16 size small letter font >>>
#define USING_SMALL_LETTER_FONT_8x16          __CONFIG_ENABLE




#endif /* end of _DISPLAY_FONT_CFG_H_ */
