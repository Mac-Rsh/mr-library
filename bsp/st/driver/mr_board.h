/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _MR_BOARD_H_
#define _MR_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_STM32F0
#include "stm32f0xx.h"
#elifdef MR_USING_STM32F1
#include "stm32f1xx.h"
#elifdef MR_USING_STM32F2
#include "stm32f2xx.h"
#elifdef MR_USING_STM32F3
#include "stm32f3xx.h"
#elifdef MR_USING_STM32F4
#include "stm32f4xx.h"
#elifdef MR_USING_STM32F7
#include "stm32f7xx.h"
#elifdef MR_USING_STM32G0
#include "stm32g0xx.h"
#elifdef MR_USING_STM32G4
#include "stm32g4xx.h"
#elifdef MR_USING_STM32H5
#include "stm32h5xx.h"
#elifdef MR_USING_STM32H7
#include "stm32h7xx.h"
#elifdef MR_USING_STM32L0
#include "stm32l0xx.h"
#elifdef MR_USING_STM32L1
#include "stm32l1xx.h"
#elifdef MR_USING_STM32L4
#include "stm32l4xx.h"
#elifdef MR_USING_STM32L5
#include "stm32l5xx.h"
#else
#error "Please define your board type in mr_board.h"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_BOARD_H_ */
