/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-28     MacRsh       first version
 */

#ifndef _MR_SOFT_IIC_H_
#define _MR_SOFT_IIC_H_

#include "mr_soft_iic_cfg.h"

#if (USING_SOFT_IIC == __CONFIG_ENABLE)

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
#if (U_SOFT_IIC_DEBUG == __CONFIG_ENABLE)
#define MR_DEBUG_ARGS_NULL(args,ret)  \
    if(args == MR_NULL) return ret
#define MR_DEBUG_ARGS_IF(args_if,ret) \
    if(args_if) return ret
#else
#define MR_DEBUG_ARGS_NULL(args,ret);
#define MR_DEBUG_ARGS_IF(args_if,ret);
#endif /* end of U_SOFT_IIC_DEBUG */
#endif /* end of _DRIVER_DEF_H_ */

#define LEVEL_HIGH                    1
#define LEVEL_LOW                     0

#define IIC_WR                        0         /* writing only */
#define IIC_RD                        1         /* reading only */

struct mr_soft_iic_msg
{
  mr_uint8_t read_write;                        /* IIC_WR/ IIC_RD */
  mr_uint8_t addr;
  mr_uint8_t *buffer;
  mr_size_t size;
};

struct mr_soft_iic
{
  mr_uint8_t addr;

  struct mr_soft_iic_bus *bus;
};

struct mr_soft_iic_bus
{
  void (*set_clk)(mr_uint8_t level);
  void (*set_sda)(mr_uint8_t level);
  mr_uint8_t (*get_sda)(void);

  struct mr_soft_iic *owner;
  mr_uint8_t lock;
};

/**
 *  Export soft-iic functions
 */
void mr_soft_iic_attach(struct mr_soft_iic *iic, struct mr_soft_iic_bus *iic_bus);
mr_err_t mr_soft_iic_transfer(struct mr_soft_iic *iic, struct mr_soft_iic_msg msg);

#endif /* end of USING_SOFT_IIC */

#endif /* end of _MR_SOFT_IIC_H_ */
