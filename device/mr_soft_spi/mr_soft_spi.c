/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-27     MacRsh       first version
 */

#include "mr_soft_spi.h"

/** user specification :
 *  please set the configuration file first !!!
 *
 *  if you want to use the soft-spi library, adapt the following api:
 *  (must to adapt) 1. struct mr_soft_spi_bus : create the spi-bus
 *  (must to adapt) 2. mr_soft_spi_bus : adapt the spi-bus functions
 *      set_clk : set the clock line to high/low level
 *      set_mosi : set the mosi line to high/low level
 *      get_miso : get the miso line level
 *  (must to adapt) 3. struct mr_soft_spi : create the spi device
 *  (must to adapt) 4. mr_soft_spi : adapt the spi device functions
 *      set_cs : set the cs line to high/low level
 *
 *  then, you can use the following api:
 *  (can use) mr_soft_spi_attach : attach spi device to spi-bus
 *  (can use) mr_soft_spi_transfer : the spi device through spi-bus to transfer the spi message
 *
 */

#if (USING_SOFT_SPI == __CONFIG_ENABLE)

/**
 * This function will attach spi device to spi-bus
 *
 * @param spi the spi device to be mounted
 * @param spi_bus the spi-bus
 *
 */
void mr_soft_spi_attach(struct mr_soft_spi *spi, struct mr_soft_spi_bus *spi_bus)
{
  /* check function args */
  MR_DEBUG_ARGS_NULL(spi,);
  MR_DEBUG_ARGS_NULL(spi_bus,);

  spi->bus = spi_bus;
}

#define mr_soft_spi_bus_delay(x)  for(mr_size_t temp = x; temp--;)

/**
 * This function will transmit data through spi-bus
 *
 * @param spi the spi device
 * @param send_buffer the send data
 *
 * @return the receive data
 *
 */
static mr_uint8_t mr_soft_spi_bus_transmit(struct mr_soft_spi *spi, mr_uint8_t send_buffer)
{
  mr_uint8_t recv_buffer = 0;
  mr_uint8_t clk_level;
  mr_uint8_t i;

  if(spi->mode == 0 || spi->mode == 1)
    clk_level = LEVEL_LOW;
  else
    clk_level = LEVEL_HIGH;

  spi->bus->set_clk(clk_level);
  clk_level = ! clk_level;

  if(spi->mode % 2 == 0)
  {
    for(i = 8; i > 0; i --)
    {
      /* send */
      if(send_buffer & 0x80)
        spi->bus->set_mosi(LEVEL_HIGH);
      else
        spi->bus->set_mosi(LEVEL_LOW);
      send_buffer <<= 1;

      mr_soft_spi_bus_delay(100);
      spi->bus->set_clk(clk_level);
      clk_level = ! clk_level;

      /* receive */
      recv_buffer <<= 1;
      recv_buffer |= spi->bus->get_miso();

      mr_soft_spi_bus_delay(100);
      spi->bus->set_clk(clk_level);
      clk_level = ! clk_level;
    }
  }
  else
  {
    for(i = 8; i > 0; i --)
    {
      spi->bus->set_clk(clk_level);
      clk_level = ! clk_level;

      /* send */
      if(send_buffer & 0x80)
        spi->bus->set_mosi(LEVEL_HIGH);
      else
        spi->bus->set_mosi(LEVEL_LOW);
      send_buffer <<= 1;

      mr_soft_spi_bus_delay(100);
      spi->bus->set_clk(clk_level);
      clk_level = ! clk_level;

      /* receive */
      recv_buffer <<= 1;
      recv_buffer |= spi->bus->get_miso();

      mr_soft_spi_bus_delay(100);
    }
  }

  return recv_buffer;
}

/**
 * This function will the spi device take the spi-bus right of use
 *
 * @param spi the spi device
 *
 * @return the error code, MR_EOK on initialization successfully.
 *
 */
static mr_err_t mr_soft_spi_bus_take(struct mr_soft_spi *spi)
{
  mr_uint8_t spi_bus_lock;
  mr_base_t level;

  /* check function args */
  MR_DEBUG_ARGS_NULL(spi,-MR_EINVAL);
  MR_DEBUG_ARGS_NULL(spi->set_cs,-MR_ERROR);

  /* check spi-bus owner */
  if(spi->bus->owner != spi)
  {
    /* check mutex lock */
    do{
      spi_bus_lock = spi->bus->lock;
    } while(spi_bus_lock != MR_UNLOCK);

    /* rtos interrupt disable*/
    level = mr_hw_interrupt_disable();

    /* lock mutex lock */
    spi->bus->lock = MR_LOCK;

    /* rtos interrupt enable*/
    mr_hw_interrupt_enable(level);

    /* stop spi cs */
    if(spi->bus->owner != MR_NULL)
      spi->bus->owner->set_cs(!spi->bus->owner->cs_active);

    /* exchange spi-bus owner */
    spi->bus->owner = spi;

    /* start spi cs */
    spi->set_cs(spi->cs_active);
  }
  else
  {
    /* rtos interrupt disable*/
    level = mr_hw_interrupt_disable();

    /* lock mutex lock */
    spi->bus->lock = MR_LOCK;

    /* rtos interrupt enable*/
    mr_hw_interrupt_enable(level);

#if (U_SOFT_SPI_BUS_CS_VALID == __CONFIG_ENABLE)
    /* start spi cs */
    spi->set_cs(spi->cs_active);
#endif
  }

  return MR_EOK;
}

/**
 * This function will the spi device release the spi-bus right of use
 *
 * @param spi the spi device
 *
 * @return the error code, MR_EOK on initialization successfully.
 *
 */
static mr_err_t mr_soft_spi_bus_release(struct mr_soft_spi *spi)
{
  /* check function args */
  MR_DEBUG_ARGS_NULL(spi,-MR_EINVAL);
  MR_DEBUG_ARGS_NULL(spi->set_cs,-MR_ERROR);

  /* check spi-bus owner */
  if(spi->bus->owner == spi)
  {
#if (U_SOFT_SPI_BUS_CS_VALID == __CONFIG_ENABLE)
    /* stop spi cs */
    spi->set_cs(!spi->cs_active);
#endif
    /* unlock mutex lock */
    spi->bus->lock = MR_UNLOCK;
    return MR_EOK;
  }

  return -MR_ERROR;
}

/**
 * This function will the spi device through spi-bus to transfer the spi message
 *
 * @param spi the spi device
 * @param msg the spi message
 *
 * @return the error code, MR_EOK on initialization successfully.
 *
 */
mr_err_t mr_soft_spi_transfer(struct mr_soft_spi *spi, struct mr_soft_spi_msg msg)
{
  mr_err_t ret;

  /* check function args */
  MR_DEBUG_ARGS_NULL(spi,-MR_EINVAL);
  MR_DEBUG_ARGS_IF(msg.read_write > SPI_WR_THEN_RD,-MR_EINVAL);

  /* take spi-bus */
  ret = mr_soft_spi_bus_take(spi);
  if(ret != MR_EOK)
    return ret;

  if(msg.read_write == SPI_WR || msg.recv_buffer == MR_NULL)
    msg.recv_size = 0;
  if(msg.read_write == SPI_RD || msg.send_buffer == MR_NULL)
    msg.send_size = 0;

  switch (msg.read_write) {
    case SPI_RD:
      /* receive */
      while (msg.recv_size) {
        *msg.recv_buffer = mr_soft_spi_bus_transmit(spi,0u);

        ++msg.recv_buffer;
        --msg.recv_size;
      }
      break;
    case SPI_WR:
      /* send */
      while (msg.send_size) {
        mr_soft_spi_bus_transmit(spi,*msg.send_buffer);

        ++msg.send_buffer;
        --msg.send_size;
      }
      break;
    case SPI_WR_THEN_RD:
      /* send */
      while (msg.send_size) {
        mr_soft_spi_bus_transmit(spi,*msg.send_buffer);

        ++msg.send_buffer;
        --msg.send_size;
      }
      /* receive */
      while (msg.recv_size) {
        *msg.recv_buffer = mr_soft_spi_bus_transmit(spi,0u);

        ++msg.recv_buffer;
        --msg.recv_size;
      }
      break;
    case SPI_RDWR:
      /* transmit */
      while (msg.send_size) {
        *msg.recv_buffer = mr_soft_spi_bus_transmit(spi,*msg.send_buffer);

        ++msg.send_buffer;
        ++msg.recv_buffer;
        --msg.send_size;
      }
      break;
  }

  /* release spi-bus */
  mr_soft_spi_bus_release(spi);

  return MR_EOK;
}

#endif /* end of USING_SOFT_SPI */
