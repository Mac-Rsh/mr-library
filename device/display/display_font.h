/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-17     MacRsh       first version
 */

#ifndef _DISPLAY_FONT_H_
#define _DISPLAY_FONT_H_

#include "display_font_cfg.h"


















#if (USING_ASCII_FONT_6x8 == __CONFIG_ENABLE)
extern const unsigned char ascii_font_6x8[][6];
#endif /* end of USING_ASCII_FONT_6x8 */

#if (USING_ASCII_FONT_8x16 == __CONFIG_ENABLE)
extern const unsigned char ascii_font_8x16[][16];
#endif  /* end of USING_8x16_FONT */

#if (USING_FIGURE_FONT_6x8 == __CONFIG_ENABLE)
extern const unsigned char figure_font_6x8[][6];
#endif  /* end of USING_FIGURE_FONT_6x8 */

#if (USING_FIGURE_FONT_8x16 == __CONFIG_ENABLE)
extern const unsigned char figure_font_8x16[][16];
#endif  /* end of USING_FIGURE_FONT_8x16 */

#if (USING_CAPITAL_LETTER_FONT_6x8 == __CONFIG_ENABLE)
extern const unsigned char capital_letter_font_6x8[][6];
#endif  /* end of USING_CAPITAL_LETTER_FONT_6x8 */

#if (USING_CAPITAL_LETTER_FONT_8x16 == __CONFIG_ENABLE)
extern const unsigned char capital_letter_font_8x16[][16];
#endif  /* end of USING_CAPITAL_LETTER_FONT_8x16 */

#if (USING_SMALL_LETTER_FONT_6x8 == __CONFIG_ENABLE)
extern const unsigned char small_letter_font_6x8[][6];
#endif  /* end of USING_SMALL_LETTER_FONT_6x8 */

#if (USING_SMALL_LETTER_FONT_8x16 == __CONFIG_ENABLE)
extern const unsigned char small_letter_font_8x16[][16];
#endif  /* end of USING_SMALL_LETTER_FONT_8x16 */



#endif /* end of _DISPLAY_FONT_H_ */
