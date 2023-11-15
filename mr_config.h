/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-10-18    MacRsh       First version
 */

#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MR_CFG_NAME_MAX                     10
#define MR_CFG_DESC_MAX                     32

#define MR_USING_RDWR_CTRL
#define MR_USING_ADC
#define MR_USING_DAC
#define MR_USING_GPIO
#define MR_USING_I2C
#define MR_USING_SPI
#define MR_USING_UART
#define MR_USING_PWM

#define MR_CFG_CONSOLE_NAME             "uart1"
#define MR_CFG_UART_RD_BUFSZ_INIT       32
#define MR_CFG_UART_WR_BUFSZ_INIT       0
#define MR_CFG_SPI_RD_BUFSZ_INIT        32
#define MR_CFG_I2C_RD_BUFSZ_INIT        32

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_CONFIG_H_ */
