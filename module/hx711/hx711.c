/*
 * @copyright (c) 2023, MR Development Team
 *
 * @license SPDX-License-Identifier: Apache-2.0
 *
 * @date 2023-11-17    MacRsh       First version
 */

#include "hx711.h"

#ifdef MR_USING_HX711

#if !defined(MR_USING_PIN)
#error "Please define MR_USING_PIN. Otherwise HX711 will not work."
#else

#include "include/device/pin.h"

static void hx711_set_sck(struct mr_hx711 *hx711, uint8_t value)
{
    mr_dev_ioctl(hx711->desc, MR_CTL_SET_OFFSET, &hx711->sck_pin);
    mr_dev_write(hx711->desc, &value, sizeof(value));
}

static uint8_t hx711_get_dout(struct mr_hx711 *hx711)
{
    uint8_t value = 0;

    mr_dev_ioctl(hx711->desc, MR_CTL_SET_OFFSET, &hx711->dout_pin);
    mr_dev_read(hx711->desc, &value, sizeof(value));
    return value;
}

static uint32_t hx711_get_value(struct mr_hx711 *hx711)
{
    uint32_t value = 0;
    int i = 0;

    /* Start the conversion */
    hx711_set_sck(hx711, 0);
    mr_delay_us(1);
    while (hx711_get_dout(hx711) != 0)
    {
        i++;
        if (i > UINT16_MAX)
        {
            return 0;
        }
    }

    /* Get the value */
    for (i = 0; i < 24; i++)
    {
        hx711_set_sck(hx711, 1);
        value <<= 1;
        mr_delay_us(1);
        hx711_set_sck(hx711, 0);
        if (hx711_get_dout(hx711) == 1)
        {
            value++;
        }
        mr_delay_us(1);
    }
    hx711_set_sck(hx711, 1);
    value = value ^ 0x800000;
    mr_delay_us(1);
    hx711_set_sck(hx711, 0);

    /* Filter the value */
    mr_bits_clr(value, (1 << hx711->filter_bits) - 1);
    return value;
}

static int mr_hx711_open(struct mr_dev *dev)
{
    struct mr_hx711 *hx711 = (struct mr_hx711 *)dev;

    hx711->desc = mr_dev_open("pin", MR_OFLAG_RDWR);
    if (hx711->desc < 0)
    {
        return hx711->desc;
    }

    /* Set the sck pin mode */
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_NUMBER, &hx711->sck_pin);
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_OUTPUT));

    /* Set the dout pin mode */
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_NUMBER, &hx711->dout_pin);
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_INPUT_UP));
    return MR_EOK;
}

static int mr_hx711_close(struct mr_dev *dev)
{
    struct mr_hx711 *hx711 = (struct mr_hx711 *)dev;

    /* Reset the sck pin mode */
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_NUMBER, &hx711->sck_pin);
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_NONE));

    /* Reset the dout pin mode */
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_NUMBER, &hx711->dout_pin);
    mr_dev_ioctl(hx711->desc, MR_CTL_PIN_SET_MODE, mr_make_local(int, MR_PIN_MODE_NONE));

    mr_dev_close(hx711->desc);
    hx711->desc = -1;
    return MR_EOK;
}

static ssize_t mr_hx711_read(struct mr_dev *dev, int off, void *buf, size_t size, int async)
{
    struct mr_hx711 *hx711 = (struct mr_hx711 *)dev;
    uint32_t *rd_buf = (uint32_t *)buf;
    ssize_t rd_size = 0;

    mr_bits_clr(size, sizeof(*rd_buf) - 1);
    for (rd_size = 0; rd_size < size; rd_size += sizeof(*rd_buf))
    {
        uint32_t value = hx711_get_value(hx711);
        *rd_buf = (value > hx711->self_cal) ? (value - hx711->self_cal) : 0;
        rd_buf++;
    }
    return rd_size;
}

static int mr_hx711_ioctl(struct mr_dev *dev, int off, int cmd, void *args)
{
    struct mr_hx711 *hx711 = (struct mr_hx711 *)dev;

    switch (cmd)
    {
        case MR_CTL_HX711_SET_FILTER_BITS:
        {
            if (args != MR_NULL)
            {
                int filter = *(int *)args;
                hx711->filter_bits = filter;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_HX711_SET_SELF_CAL:
        {
            hx711->self_cal = hx711_get_value(hx711);
            return MR_EOK;
        }

        case MR_CTL_HX711_GET_FILTER_BITS:
        {
            if (args != MR_NULL)
            {
                *(int *)args = hx711->filter_bits;
                return MR_EOK;
            }
            return MR_EINVAL;
        }
        case MR_CTL_HX711_GET_SELF_CAL:
        {
            if (args != MR_NULL)
            {
                *(uint32_t *)args = hx711->self_cal;
                return MR_EOK;
            }
            return MR_EINVAL;
        }

        default:
        {
            return MR_ENOTSUP;
        }
    }
}

/**
 * @brief This function register a hx711 module.
 *
 * @param hx711 The hx711 module.
 * @param name The name of the hx711 module.
 * @param sck_pin The sck pin of the hx711 module.
 * @param dout_pin The dout pin of the hx711 module.
 *
 * @return MR_EOK on success, otherwise an error code.
 */
int mr_hx711_register(struct mr_hx711 *hx711, const char *name, int sck_pin, int dout_pin)
{
    static struct mr_dev_ops ops =
        {
            mr_hx711_open,
            mr_hx711_close,
            mr_hx711_read,
            MR_NULL,
            mr_hx711_ioctl,
            MR_NULL
        };

    mr_assert(hx711 != MR_NULL);
    mr_assert(name != MR_NULL);
    mr_assert(sck_pin >= 0);
    mr_assert(dout_pin >= 0);

    /* Initialize the fields */
    hx711->filter_bits = 0;
    hx711->self_cal = 0;
    hx711->sck_pin = sck_pin;
    hx711->dout_pin = dout_pin;
    hx711->desc = -1;

    /* Register the hx711 */
    return mr_dev_register(&hx711->dev, name, Mr_Dev_Type_ADC, MR_SFLAG_RDONLY | MR_SFLAG_NONDRV, &ops, MR_NULL);
}

#endif /* MR_USING_PIN */

#endif /* MR_USING_HX711 */
