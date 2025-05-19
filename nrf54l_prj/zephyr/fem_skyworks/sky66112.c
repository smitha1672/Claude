/**
 * Copyright (c) 2025, Motive
 *
 * SPDX-License-Identifier: Proprietary
 */

/**
 * @file sky66112.c
 * @brief Driver for Skyworks SKY66112-11 RF front-end module
 *
 * This module provides control of the SKY66112-11 FEM for the Beacon V2 project
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include "sky66112.h"

LOG_MODULE_REGISTER(sky66112, CONFIG_SKY66112_LOG_LEVEL);

/* Timing constants (in microseconds) */
#define SKY66112_T_CSD_SETUP_US   10  /* CSD setup time */
#define SKY66112_T_MODE_SETUP_US   5  /* Mode switch setup time */

/**
 * @brief Configure all GPIO pins for the SKY66112
 *
 * @param dev Device pointer to the SKY66112 device
 * @return 0 if successful, negative errno code otherwise
 */
static int _configure_gpios(const struct device *dev)
{
    const struct sky66112_config *config = dev->config;
    int ret;

    /* Configure CSD GPIO */
    if (!gpio_is_ready_dt(&config->csd_gpio)) {
        LOG_ERR("CSD GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&config->csd_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure CSD GPIO: %d", ret);
        return ret;
    }

    /* Configure CPS GPIO */
    if (!gpio_is_ready_dt(&config->cps_gpio)) {
        LOG_ERR("CPS GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&config->cps_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure CPS GPIO: %d", ret);
        return ret;
    }

    /* Configure CRX GPIO */
    if (!gpio_is_ready_dt(&config->crx_gpio)) {
        LOG_ERR("CRX GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&config->crx_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure CRX GPIO: %d", ret);
        return ret;
    }

    /* Configure CTX GPIO */
    if (!gpio_is_ready_dt(&config->ctx_gpio)) {
        LOG_ERR("CTX GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&config->ctx_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure CTX GPIO: %d", ret);
        return ret;
    }

    /* Configure CHL GPIO */
    if (!gpio_is_ready_dt(&config->chl_gpio)) {
        LOG_ERR("CHL GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&config->chl_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure CHL GPIO: %d", ret);
        return ret;
    }

    /* Configure ANT_SEL GPIO */
    if (!gpio_is_ready_dt(&config->ant_sel_gpio)) {
        LOG_ERR("ANT_SEL GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&config->ant_sel_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure ANT_SEL GPIO: %d", ret);
        return ret;
    }

    return 0;
}

int sky66112Init(const struct device *dev)
{
    struct sky66112_data *data = dev->data;
    int ret;

    /* Configure all GPIOs */
    ret = _configure_gpios(dev);
    if (ret < 0) {
        return ret;
    }

    /* Set initial state (sleep mode) */
    data->current_mode = SKY66112_MODE_SLEEP;
    data->current_antenna = SKY66112_ANT1;
    
    LOG_INF("SKY66112 initialized");
    return 0;
}

int sky66112SetMode(const struct device *dev, Sky66112Mode mode)
{
    const struct sky66112_config *config = dev->config;
    struct sky66112_data *data = dev->data;
    int ret;

    /* First, ensure the device is enabled by setting CSD high */
    ret = gpio_pin_set_dt(&config->csd_gpio, 1);
    if (ret < 0) {
        LOG_ERR("Failed to enable SKY66112 (CSD): %d", ret);
        return ret;
    }
    k_busy_wait(SKY66112_T_CSD_SETUP_US);

    /* Configure mode-specific pins */
    switch (mode) {
    case SKY66112_MODE_SLEEP:
        /* Turn everything off for sleep mode */
        ret = gpio_pin_set_dt(&config->cps_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        ret = gpio_pin_set_dt(&config->crx_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        ret = gpio_pin_set_dt(&config->ctx_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        ret = gpio_pin_set_dt(&config->chl_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        ret = gpio_pin_set_dt(&config->csd_gpio, 0);  /* Finally, disable the chip */
        if (ret < 0) {
            return ret;
        }
        break;

    case SKY66112_MODE_RX:
        /* Set bypass mode (CPS low) */
        ret = gpio_pin_set_dt(&config->cps_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        /* Disable TX */
        ret = gpio_pin_set_dt(&config->ctx_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        /* Set power mode low for RX */
        ret = gpio_pin_set_dt(&config->chl_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        k_busy_wait(SKY66112_T_MODE_SETUP_US);
        /* Enable RX */
        ret = gpio_pin_set_dt(&config->crx_gpio, 1);
        if (ret < 0) {
            return ret;
        }
        break;

    case SKY66112_MODE_TX_LOW:
        /* Disable RX */
        ret = gpio_pin_set_dt(&config->crx_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        /* Set power mode low for low-power TX */
        ret = gpio_pin_set_dt(&config->chl_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        /* Set PA mode (CPS high) */
        ret = gpio_pin_set_dt(&config->cps_gpio, 1);
        if (ret < 0) {
            return ret;
        }
        k_busy_wait(SKY66112_T_MODE_SETUP_US);
        /* Enable TX */
        ret = gpio_pin_set_dt(&config->ctx_gpio, 1);
        if (ret < 0) {
            return ret;
        }
        break;

    case SKY66112_MODE_TX_HIGH:
        /* Disable RX */
        ret = gpio_pin_set_dt(&config->crx_gpio, 0);
        if (ret < 0) {
            return ret;
        }
        /* Set power mode high for high-power TX */
        ret = gpio_pin_set_dt(&config->chl_gpio, 1);
        if (ret < 0) {
            return ret;
        }
        /* Set PA mode (CPS high) */
        ret = gpio_pin_set_dt(&config->cps_gpio, 1);
        if (ret < 0) {
            return ret;
        }
        k_busy_wait(SKY66112_T_MODE_SETUP_US);
        /* Enable TX */
        ret = gpio_pin_set_dt(&config->ctx_gpio, 1);
        if (ret < 0) {
            return ret;
        }
        break;

    default:
        LOG_ERR("Invalid mode: %d", mode);
        return -EINVAL;
    }

    data->current_mode = mode;
    LOG_DBG("SKY66112 mode set to %d", mode);
    return 0;
}

Sky66112Mode sky66112GetMode(const struct device *dev)
{
    struct sky66112_data *data = dev->data;
    return data->current_mode;
}

int sky66112SelectAntenna(const struct device *dev, Sky66112Antenna antenna)
{
    const struct sky66112_config *config = dev->config;
    struct sky66112_data *data = dev->data;
    int ret;

    /* Set antenna selection pin */
    ret = gpio_pin_set_dt(&config->ant_sel_gpio, antenna == SKY66112_ANT2);
    if (ret < 0) {
        LOG_ERR("Failed to set antenna selection: %d", ret);
        return ret;
    }

    data->current_antenna = antenna;
    LOG_DBG("SKY66112 antenna set to %d", antenna);
    return 0;
}

Sky66112Antenna sky66112GetAntenna(const struct device *dev)
{
    struct sky66112_data *data = dev->data;
    return data->current_antenna;
}

uint8_t sky66112GetTxGain(const struct device *dev)
{
    struct sky66112_data *data = dev->data;

    if (data->current_mode == SKY66112_MODE_TX_HIGH) {
        return 21; /* Fixed value for high-power mode: +21 dBm */
    } else if (data->current_mode == SKY66112_MODE_TX_LOW) {
        return 16; /* Fixed value for low-power mode: +16 dBm */
    } else {
        return 0; /* Not in TX mode */
    }
}

uint8_t sky66112GetRxGain(const struct device *dev)
{
    struct sky66112_data *data = dev->data;

    if (data->current_mode == SKY66112_MODE_RX) {
        return 11; /* Fixed value for RX mode: +11 dB */
    } else {
        return 0; /* Not in RX mode */
    }
}

int sky66112PmControl(const struct device *dev, enum pm_device_action action)
{
    int ret = 0;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        /* Resume operation by initializing again */
        ret = sky66112Init(dev);
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        /* Suspend operation by setting to sleep mode */
        ret = sky66112SetMode(dev, SKY66112_MODE_SLEEP);
        break;
    default:
        ret = -ENOTSUP;
    }

    return ret;
}

#define SKY66112_INIT(inst)                                                    \
    static const struct sky66112_config sky66112_config_##inst = {            \
        .csd_gpio = GPIO_DT_SPEC_INST_GET(inst, csd_gpios),                  \
        .cps_gpio = GPIO_DT_SPEC_INST_GET(inst, cps_gpios),                  \
        .crx_gpio = GPIO_DT_SPEC_INST_GET(inst, crx_gpios),                  \
        .ctx_gpio = GPIO_DT_SPEC_INST_GET(inst, ctx_gpios),                  \
        .chl_gpio = GPIO_DT_SPEC_INST_GET(inst, chl_gpios),                  \
        .ant_sel_gpio = GPIO_DT_SPEC_INST_GET(inst, ant_sel_gpios),          \
    };                                                                        \
                                                                              \
    static struct sky66112_data sky66112_data_##inst = {                      \
        .current_mode = SKY66112_MODE_SLEEP,                                 \
        .current_antenna = SKY66112_ANT1,                                    \
    };                                                                        \
                                                                              \
    PM_DEVICE_DT_INST_DEFINE(inst, sky66112PmControl);                       \
                                                                              \
    DEVICE_DT_INST_DEFINE(inst,                                              \
                        sky66112Init,                                        \
                        PM_DEVICE_DT_INST_GET(inst),                         \
                        &sky66112_data_##inst,                               \
                        &sky66112_config_##inst,                             \
                        POST_KERNEL,                                         \
                        CONFIG_SKY66112_INIT_PRIORITY,                       \
                        NULL);

DT_INST_FOREACH_STATUS_OKAY(SKY66112_INIT)
