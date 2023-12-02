/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-10    MacRsh       First version
 */

#ifndef _DRV_SPI_H_
#define _DRV_SPI_H_

#include "include/device/spi.h"
#include "mr_board.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MR_USING_SPI

struct drv_spi_bus_data
{
    SPI_HandleTypeDef handle;
    SPI_TypeDef *instance;
    IRQn_Type irq;
};

#endif /* MR_USING_SPI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DRV_SPI_H_ */
