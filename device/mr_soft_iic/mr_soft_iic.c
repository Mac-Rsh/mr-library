/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-28     MacRsh       first version
 */

#include "mr_soft_iic.h"

/** user specification :
 *  please set the configuration file first !!!
 *
 *  if you want to use the soft-iic library, adapt the following api:
 *  (must to adapt) 1. struct mr_soft_iic_bus : create the iic-bus
 *  (must to adapt) 2. mr_soft_iic_bus : adapt the iic-bus functions
 *      set_clk : set the clock line to high/low level
 *      set_sda : set the data line to high/low level
 *      get_sda : get the data line level
 *  (must to adapt) 3. struct mr_soft_iic : create the iic device
 *  (must to adapt) 4. mr_soft_iic : adapt the iic device functions
 *      addr : set the iic device addr
 *
 *  then, you can use the following api:
 *  (can use) mr_soft_iic_attach : attach iic device to iic-bus
 *  (can use) mr_soft_iic_transfer : the iic device through iic-bus to transfer the iic message
 *
 */

#if (USING_SOFT_IIC == __CONFIG_ENABLE)

/**
 * This function will attach spi device to iic-bus
 *
 * @param iic the iic device to be mounted
 * @param iic_bus the iic-bus
 *
 */
void mr_soft_iic_attach(struct mr_soft_iic *iic, struct mr_soft_iic_bus *iic_bus)
{
  /* check function args */
  MR_DEBUG_ARGS_NULL(iic,);
  MR_DEBUG_ARGS_NULL(iic_bus,);

  iic->bus = iic_bus;
}

#define mr_soft_iic_bus_delay(x)  for(mr_size_t temp = x; temp--;)

/**
 * This function will send iic start signal
 *
 * @param iic_bus the iic-bus
 *
 */
static void mr_soft_iic_bus_start(struct mr_soft_iic_bus *iic_bus)
{
  iic_bus->set_clk(LEVEL_HIGH);
  iic_bus->set_sda(LEVEL_HIGH);

  mr_soft_iic_bus_delay(100);
  iic_bus->set_sda(LEVEL_LOW);
  mr_soft_iic_bus_delay(100);
  iic_bus->set_clk(LEVEL_LOW);
}

/**
 * This function will send iic stop signal
 *
 * @param iic_bus the iic-bus
 *
 */
static void mr_soft_iic_bus_stop(struct mr_soft_iic_bus *iic_bus)
{
  iic_bus->set_sda(LEVEL_LOW);
  iic_bus->set_clk(LEVEL_LOW);

  mr_soft_iic_bus_delay(100);
  iic_bus->set_clk(LEVEL_HIGH);
  mr_soft_iic_bus_delay(100);
  iic_bus->set_sda(LEVEL_HIGH);
}

/**
 * This function will send iic ack signal
 *
 * @param iic_bus the iic-bus
 * @param ack the ack signal
 *
 */
static void mr_soft_iic_bus_send_ack(struct mr_soft_iic_bus *iic_bus, mr_uint8_t ack)
{
  iic_bus->set_clk(LEVEL_LOW);

  mr_soft_iic_bus_delay(100);
  if(ack)
    iic_bus->set_sda(LEVEL_HIGH);
  else
    iic_bus->set_sda(LEVEL_LOW);

  iic_bus->set_clk(LEVEL_HIGH);
  mr_soft_iic_bus_delay(100);
  iic_bus->set_clk(LEVEL_LOW);
  mr_soft_iic_bus_delay(100);
}

/**
 * This function will wait iic ack signal
 *
 * @param iic_bus the iic-bus
 *
 */
static mr_uint8_t mr_soft_iic_bus_wait_ack(struct mr_soft_iic_bus *iic_bus)
{
  mr_uint8_t ret;

  iic_bus->set_clk(LEVEL_LOW);
  mr_soft_iic_bus_delay(100);
  iic_bus->set_clk(LEVEL_HIGH);
  mr_soft_iic_bus_delay(100);

  ret = iic_bus->get_sda();

  iic_bus->set_clk(LEVEL_LOW);
  mr_soft_iic_bus_delay(100);

  return ret;
}

/**
 * This function will iic send data
 *
 * @param iic_bus the iic-bus
 * @param send_data the iic-bus send data
 *
 */
static void mr_soft_iic_bus_send(struct mr_soft_iic_bus *iic_bus, mr_uint8_t send_data)
{
  mr_uint8_t i = 8;

  while(i--)
  {
    if(send_data & 0x80)
      iic_bus->set_sda(LEVEL_HIGH);
    else
      iic_bus->set_sda(LEVEL_LOW);
    send_data = send_data << 1;

    mr_soft_iic_bus_delay(100);
    iic_bus->set_clk(LEVEL_HIGH);
    mr_soft_iic_bus_delay(100);
    iic_bus->set_clk(LEVEL_LOW);
  }

  mr_soft_iic_bus_wait_ack(iic_bus);
}

/**
 * This function will iic receive data
 *
 * @param iic_bus the iic-bus
 * @param ack the ack signal
 *
 * @return the receive data
 *
 */
static mr_uint8_t mr_soft_iic_bus_receive(struct mr_soft_iic_bus *iic_bus, mr_uint8_t ack)
{
  mr_uint8_t recv_buffer = 0;
  mr_uint8_t i = 8;

  iic_bus->set_clk(LEVEL_LOW);
  mr_soft_iic_bus_delay(100);
  iic_bus->set_sda(LEVEL_HIGH);

  while (i--)
  {
    mr_soft_iic_bus_delay(100);
    iic_bus->set_clk(LEVEL_LOW);
    mr_soft_iic_bus_delay(100);
    iic_bus->set_clk(LEVEL_HIGH);
    mr_soft_iic_bus_delay(100);
    recv_buffer = recv_buffer << 1;
    if(iic_bus->get_sda() == 1)
      recv_buffer |= 1;
  }

  iic_bus->set_clk(LEVEL_LOW);
  mr_soft_iic_bus_delay(100);
  mr_soft_iic_bus_send_ack(iic_bus,ack);

  return recv_buffer;
}

/**
 * This function will the iic device take the iic-bus right of use
 *
 * @param iic the iic-bus
 *
 * @return the error code, MR_EOK on initialization successfully.
 *
 */
static mr_err_t mr_soft_iic_bus_take(struct mr_soft_iic *iic)
{
  mr_uint8_t iic_bus_lock;
  mr_base_t level;

  /* check function args */
  MR_DEBUG_ARGS_NULL(iic,-MR_EINVAL);

  /* check iic-bus owner */
  if(iic->bus->owner != iic)
  {
    /* check mutex lock */
    do {
      iic_bus_lock = iic->bus->lock;
    } while (iic_bus_lock != MR_UNLOCK);

    /* rtos interrupt disable*/
    level = mr_hw_interrupt_disable();

    /* lock mutex lock */
    iic->bus->lock = MR_LOCK;

    /* rtos interrupt enable*/
    mr_hw_interrupt_enable(level);

    /* exchange iic-bus owner */
    iic->bus->owner = iic;
  }
  else
  {
    /* rtos interrupt disable*/
    level = mr_hw_interrupt_disable();

    /* lock mutex lock */
    iic->bus->lock = MR_LOCK;

    /* rtos interrupt enable*/
    mr_hw_interrupt_enable(level);
  }

  return MR_EOK;
}

/**
 * This function will the iic device release the iic-bus right of use
 *
 * @param iic the iic-bus
 *
 * @return the error code, MR_EOK on initialization successfully.
 *
 */
static mr_err_t mr_soft_iic_bus_release(struct mr_soft_iic *iic)
{
  /* check function args */
  MR_DEBUG_ARGS_NULL(iic,-MR_EINVAL);

  /* check spi-bus owner */
  if(iic->bus->owner == iic)
  {
    iic->bus->lock = MR_UNLOCK;
    return MR_EOK;
  }

  return -MR_ERROR;
}

/**
 * This function will the iic device through iic-bus to transfer the iic message
 *
 * @param iic the iic device
 * @param msg the iic message
 *
 * @return the error code, MR_EOK on initialization successfully.
 *
 */
mr_err_t mr_soft_iic_transfer(struct mr_soft_iic *iic, struct mr_soft_iic_msg msg)
{
  mr_err_t ret;

  /* check msg */
  if(msg.read_write > IIC_RD)
    return -MR_ERROR;
  if(msg.addr == MR_NULL)
    return -MR_EINVAL;
  if(msg.buffer == MR_NULL || msg.size == MR_NULL)
    return MR_EOK;

  /* take iic-bus */
  ret = mr_soft_iic_bus_take(iic);
  if(ret != MR_EOK)
    return ret;

  /* send iic device and register address */
  mr_soft_iic_bus_start(iic->bus);
  mr_soft_iic_bus_send(iic->bus, iic->addr << 1);
  mr_soft_iic_bus_send(iic->bus, msg.addr);

  if(msg.read_write == IIC_WR)
  {
    /* send iic start and device write cmd */
    mr_soft_iic_bus_start(iic->bus);
    mr_soft_iic_bus_send(iic->bus, iic->addr << 1);

    /* send */
    while(msg.size)
    {
      mr_soft_iic_bus_send(iic->bus,*msg.buffer);

      ++ msg.buffer;
      -- msg.size;
    }

    /* send iic stop */
    mr_soft_iic_bus_stop(iic->bus);
  }
  else
  {
    /* send iic start and device write cmd */
    mr_soft_iic_bus_start(iic->bus);
    mr_soft_iic_bus_send(iic->bus, iic->addr << 1 | 0x01);

    /* receive */
    while(msg.size)
    {
      *msg.buffer = mr_soft_iic_bus_receive(iic->bus, (msg.size == 0));

      ++ msg.buffer;
      -- msg.size;
    }
    /* send iic stop */
    mr_soft_iic_bus_stop(iic->bus);
  }

  /* release iic-bus */
  mr_soft_iic_bus_release(iic);

  return MR_EOK;
}


#endif /* end of USING_SOFT_IIC */