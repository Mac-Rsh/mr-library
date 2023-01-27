/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-14     MacRsh       first version
 */

#ifndef _SOFT_SPI_H_
#define _SOFT_SPI_H_

#include "mr_soft_spi_cfg.h"

#if (USING_SOFT_SPI == __CONFIG_ENABLE)

//<<< base data type(please check that there is no conflict then change data type to typedef) >>>
#ifndef _DRIVER_DEF_H_
typedef unsigned char                 mr_uint8_t;
typedef unsigned int                  mr_size_t;
typedef long                          mr_base_t;
typedef mr_base_t                     mr_err_t;
#define MR_NULL                       0
#define MR_EOK                        0           /**< There is no error */
#define MR_ERROR                      1           /**< A generic error happens */
#define MR_EINVAL                     10          /**< Invalid argument */
/* lock type definitions */
#define MR_LOCK                       1           /**< lock */
#define MR_UNLOCK                     0           /**< unlock */
/* debug */
#if (U_SOFT_SPI_DEBUG == __CONFIG_ENABLE)
#define MR_DEBUG_ARGS_NULL(args,ret)  \
    if(args == MR_NULL) return ret
#define MR_DEBUG_ARGS_IF(args_if,ret) \
    if(args_if) return ret
#else
#define MR_DEBUG_ARGS_NULL(args,ret);
#define MR_DEBUG_ARGS_IF(args_if,ret);
#endif /* end of U_SOFT_SPI_DEBUG */
#endif /* end of _DRIVER_DEF_H_ */

#define LEVEL_HIGH                    1
#define LEVEL_LOW                     0

#define SPI_MODE_0                    0           /* CPOL = 0, CPHA = 0 */
#define SPI_MODE_1                    1           /* CPOL = 0, CPHA = 1 */
#define SPI_MODE_2                    2           /* CPOL = 1, CPHA = 0 */
#define SPI_MODE_3                    3           /* CPOL = 1, CPHA = 1 */

#define SPI_WR                        0           /* writing only */
#define SPI_RD                        1           /* reading only */
#define SPI_RDWR                      2           /* writing-while-reading */
#define SPI_WR_THEN_RD                3           /* writing first, reading later */

struct mr_soft_spi_msg
{
  mr_uint8_t read_write;                         /* SOFT_SPI_WR/ SOFT_SPI_RD/ SOFT_SPI_RDWR/ SOFT_SPI_WR_THEN_RD */
  mr_uint8_t *send_buffer;
  mr_size_t send_size;
  mr_uint8_t *recv_buffer;
  mr_size_t recv_size;
};

struct mr_soft_spi
{
  mr_uint8_t mode         :2;
  mr_uint8_t cs_active    :1;
  struct mr_soft_spi_bus *bus;

  void (*set_cs)(mr_uint8_t level);
};


struct mr_soft_spi_bus
{
  void (*set_clk)(mr_uint8_t level);
  void (*set_mosi)(mr_uint8_t level);
  mr_uint8_t (*get_miso)(void);

  struct mr_soft_spi *owner;
  mr_uint8_t lock;
};

/**
 *  Export soft-spi functions
 */
void mr_soft_spi_attach(struct mr_soft_spi *spi, struct mr_soft_spi_bus *spi_bus);
mr_err_t mr_soft_spi_transfer(struct mr_soft_spi *spi, struct mr_soft_spi_msg msg);

#endif /* end of USING_SOFT_SPI */

#endif /* end of _SOFT_SPI_H_ */
