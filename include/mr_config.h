#ifndef _MR_CONFIG_H_
#define _MR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MR_CFG_PRINTF_BUFSZ 128
#define MR_CFG_PRINTF_NAME "serial1"
#define MR_USE_LOG_ERROR
#define MR_USE_LOG_WARN
#define MR_USE_LOG_INFO
#define MR_USE_LOG_DEBUG
#define MR_USE_ASSERT
#define MR_CFG_HEAP_SIZE 4096
#define MR_CFG_DEVICE_NAME_MAX 12
#define MR_CFG_DESCRIPTOR_MAX 128
#define MR_USE_SERIAL
#define MR_CFG_SERIAL_RD_FIFO_SIZE 64
#define MR_CFG_SERIAL_WR_FIFO_SIZE 0

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MR_CONFIG_H_ */
