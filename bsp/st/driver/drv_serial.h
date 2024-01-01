/*
 * @copyright (c) 2023-2024, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _DRV_SERIAL_H_
#define _DRV_SERIAL_H_

#include "include/device/mr_serial.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SERIAL

struct drv_serial_data
{
    UART_HandleTypeDef handle;
    USART_TypeDef *instance;
    IRQn_Type irq;
};

#endif /* MR_USING_SERIAL */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_SERIAL_H_ */
