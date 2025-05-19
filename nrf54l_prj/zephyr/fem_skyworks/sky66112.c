/*
 * @file    sky66112.c
 * @brief   Driver for Skyworks SKY66112-11 FEM used in Beacon V2
 *
 * @copyright Copyright (c) 2025 Motive Technologies, Inc.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <fem/sky66112.h>

LOG_MODULE_REGISTER(sky66112, CONFIG_SKY66112_LOG_LEVEL);

/* Static global variables (using camelCase, no prefix) */
static const struct device *femDevice;
static const struct gpio_dt_spec csdPin;
static const struct gpio_dt_spec cpsPin;
static const struct gpio_dt_spec crxPin;
static const struct gpio_dt_spec ctxPin;
static const struct gpio_dt_spec chlPin;
static const struct gpio_dt_spec antSelPin;

/**
 * @brief Initialize the SKY66112-11 FEM.
 *
 * This function configures the GPIOs connected to the SKY66112-11 FEM
 * and sets the initial state to sleep mode.
 *
 * @return 0 on success, negative error code otherwise
 */
int sky66112Init(void)
{
    int err;

    /* Get device from device tree */
    femDevice = DEVICE_DT_GET(DT_NODELABEL(sky66112_fem));
    if (!device_is_ready(femDevice)) {
        LOG_ERR("SKY66112-11 FEM device not ready");
        return -ENODEV;
    }

    /* Configure GPIO pins */
    struct gpio_dt_spec csdPin = GPIO_DT_SPEC_GET(DT_NODELABEL(sky66112_fem), csd_gpios);
    struct gpio_dt_spec cpsPin = GPIO_DT_SPEC_GET(DT_NODELABEL(sky66112_fem), cps_gpios);
    struct gpio_dt_spec crxPin = GPIO_DT_SPEC_GET(DT_NODELABEL(sky66112_fem), crx_gpios);
    struct gpio_dt_spec ctxPin = GPIO_DT_SPEC_GET(DT_NODELABEL(sky66112_fem), ctx_gpios);
    struct gpio_dt_spec chlPin = GPIO_DT_SPEC_GET(DT_NODELABEL(sky66112_fem), chl_gpios);
    struct gpio_dt_spec antSelPin = GPIO_DT_SPEC_GET(DT_NODELABEL(sky66112_fem), ant_sel_gpios);

    /* Initialize pins as outputs */
    err = gpio_pin_configure_dt(&csdPin, GPIO_OUTPUT_INACTIVE);
    if (err) {
        LOG_ERR("Failed to configure CSD pin (err %d)", err);
        return err;
    }

    err = gpio_pin_configure_dt(&cpsPin, GPIO_OUTPUT_INACTIVE);
    if (err) {
        LOG_ERR("Failed to configure CPS pin (err %d)", err);
        return err;
    }

    err = gpio_pin_configure_dt(&crxPin, GPIO_OUTPUT_INACTIVE);
    if (err) {
        LOG_ERR("Failed to configure CRX pin (err %d)", err);
        return err;
    }

    err = gpio_pin_configure_dt(&ctxPin, GPIO_OUTPUT_INACTIVE);
    if (err) {
        LOG_ERR("Failed to configure CTX pin (err %d)", err);
        return err;
    }

    err = gpio_pin_configure_dt(&chlPin, GPIO_OUTPUT_INACTIVE);
    if (err) {
        LOG_ERR("Failed to configure CHL pin (err %d)", err);
        return err;
    }

    err = gpio_pin_configure_dt(&antSelPin, GPIO_OUTPUT_INACTIVE);
    if (err) {
        LOG_ERR("Failed to configure ANT_SEL pin (err %d)", err);
        return err;
    }

    /* Set to sleep mode by default */
    err = sky66112SetMode(SKY66112_MODE_SLEEP);
    if (err) {
        LOG_ERR("Failed to set FEM to sleep mode (err %d)", err);
        return err;
    }

    LOG_INF("SKY66112-11 FEM initialized successfully");
    return 0;
}

/**
 * @brief Set the operating mode of the SKY66112-11 FEM.
 *
 * @param mode Operating mode (see SKY66112_MODE_* defines)
 * @return 0 on success, negative error code otherwise
 */
int sky66112SetMode(enum sky66112_mode mode)
{
    int err;

    switch (mode) {
    case SKY66112_MODE_SLEEP:
        /* Mode 0: All off (sleep mode) - CSD=0, others don't matter */
        err = gpio_pin_set_dt(&csdPin, 0);
        if (err) {
            LOG_ERR("Failed to set CSD pin (err %d)", err);
            return err;
        }
        LOG_DBG("FEM set to sleep mode");
        break;

    case SKY66112_MODE_RX_LNA:
        /* Mode 1: Receive LNA mode - CSD=1, CPS=0, CRX=1, CTX=0 */
        err = gpio_pin_set_dt(&cpsPin, 0);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&crxPin, 1);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&ctxPin, 0);
        if (err) {
            return err;
        }

        /* Wake up device last */
        err = gpio_pin_set_dt(&csdPin, 1);
        if (err) {
            return err;
        }

        LOG_DBG("FEM set to RX LNA mode");
        break;

    case SKY66112_MODE_TX_HIGH_POWER:
        /* Mode 2: Transmit high-power mode - CSD=1, CPS=0, CTX=1, CHL=1 */
        err = gpio_pin_set_dt(&cpsPin, 0);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&chlPin, 1);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&ctxPin, 1);
        if (err) {
            return err;
        }

        /* Wake up device last */
        err = gpio_pin_set_dt(&csdPin, 1);
        if (err) {
            return err;
        }

        LOG_DBG("FEM set to TX high-power mode");
        break;

    case SKY66112_MODE_TX_LOW_POWER:
        /* Mode 3: Transmit low-power mode - CSD=1, CPS=0, CTX=1, CHL=0 */
        err = gpio_pin_set_dt(&cpsPin, 0);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&chlPin, 0);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&ctxPin, 1);
        if (err) {
            return err;
        }

        /* Wake up device last */
        err = gpio_pin_set_dt(&csdPin, 1);
        if (err) {
            return err;
        }

        LOG_DBG("FEM set to TX low-power mode");
        break;

    case SKY66112_MODE_RX_BYPASS:
        /* Mode 4: Receive bypass mode - CSD=1, CPS=1, CRX=1, CTX=0 */
        err = gpio_pin_set_dt(&cpsPin, 1);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&crxPin, 1);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&ctxPin, 0);
        if (err) {
            return err;
        }

        /* Wake up device last */
        err = gpio_pin_set_dt(&csdPin, 1);
        if (err) {
            return err;
        }

        LOG_DBG("FEM set to RX bypass mode");
        break;

    case SKY66112_MODE_TX_BYPASS:
        /* Mode 5: Transmit bypass mode - CSD=1, CPS=1, CTX=1 */
        err = gpio_pin_set_dt(&cpsPin, 1);
        if (err) {
            return err;
        }

        err = gpio_pin_set_dt(&ctxPin, 1);
        if (err) {
            return err;
        }

        /* Wake up device last */
        err = gpio_pin_set_dt(&csdPin, 1);
        if (err) {
            return err;
        }

        LOG_DBG("FEM set to TX bypass mode");
        break;

    default:
        LOG_ERR("Invalid FEM mode: %d", mode);
        return -EINVAL;
    }

    return 0;
}

/**
 * @brief Select the active antenna for the SKY66112-11 FEM.
 *
 * @param antenna Antenna selection (SKY66112_ANTENNA_1 or SKY66112_ANTENNA_2)
 * @return 0 on success, negative error code otherwise
 */
int sky66112SelectAntenna(enum sky66112_antenna antenna)
{
    int err;

    switch (antenna) {
    case SKY66112_ANTENNA_1:
        /* ANT_SEL = 0 for ANT1 */
        err = gpio_pin_set_dt(&antSelPin, 0);
        if (err) {
            LOG_ERR("Failed to select antenna 1 (err %d)", err);
            return err;
        }
        LOG_DBG("Selected antenna 1");
        break;

    case SKY66112_ANTENNA_2:
        /* ANT_SEL = 1 for ANT2 */
        err = gpio_pin_set_dt(&antSelPin, 1);
        if (err) {
            LOG_ERR("Failed to select antenna 2 (err %d)", err);
            return err;
        }
        LOG_DBG("Selected antenna 2");
        break;

    default:
        LOG_ERR("Invalid antenna selection: %d", antenna);
        return -EINVAL;
    }

    return 0;
}
