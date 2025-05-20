/**
 * @file sky66112.c
 * @brief Driver for the SKY66112 FEM on nRF54L15 platform.
 *
 * Configures all FEM control GPIOs in the PRE_KERNEL_1 phase for early enable
 * and provides APIs to set the FEM into various operating and antenna modes.
 *
 * Copyright (c) 2025 Motive
 */

#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sky66112, LOG_LEVEL_INF);

/* Device-tree node for SKY66112 FEM */
#define FEM_NODE DT_NODELABEL(sky66112_fem)

/* GPIO specs pulled from DT */
static const struct gpio_dt_spec fem_cps     = GPIO_DT_SPEC_GET(FEM_NODE, cps_gpios);
static const struct gpio_dt_spec fem_ctx     = GPIO_DT_SPEC_GET(FEM_NODE, ctx_gpios);
static const struct gpio_dt_spec fem_chl     = GPIO_DT_SPEC_GET(FEM_NODE, chl_gpios);
static const struct gpio_dt_spec fem_ant_sel = GPIO_DT_SPEC_GET(FEM_NODE, ant_sel_gpios);
static const struct gpio_dt_spec fem_csd     = GPIO_DT_SPEC_GET(FEM_NODE, csd_gpios);
static const struct gpio_dt_spec fem_crx     = GPIO_DT_SPEC_GET(FEM_NODE, crx_gpios);

/**
 * @brief Pre-kernel initialization for SKY66112 FEM control pins
 *
 * Configures all FEM control lines as outputs (inactive) before kernel startup.
 */
static int _sky66112_init(void)
{
    LOG_INF("Initializing SKY66112 FEM (pre-kernel)");

    /* Verify each GPIO port is ready */
    if (!device_is_ready(fem_cps.port) ||
        !device_is_ready(fem_ctx.port) ||
        !device_is_ready(fem_chl.port) ||
        !device_is_ready(fem_ant_sel.port) ||
        !device_is_ready(fem_csd.port) ||
        !device_is_ready(fem_crx.port)) {
        LOG_ERR("One or more GPIO ports not ready");
        return -ENODEV;
    }

    /* Configure each pin as output, inactive (low) */
    gpio_pin_configure_dt(&fem_cps, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&fem_ctx, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&fem_chl, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&fem_ant_sel, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&fem_csd, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&fem_crx, GPIO_OUTPUT_INACTIVE);

    LOG_INF("SKY66112 FEM control pins configured (shutdown state)");

    //FIXME:
    return 0;

    /* Bring FEM out of shutdown */
    gpio_pin_set_dt(&fem_csd, 1);
    gpio_pin_set_dt(&fem_cps, 0);
    gpio_pin_set_dt(&fem_crx, 0);
    gpio_pin_set_dt(&fem_ctx, 1);
    gpio_pin_set_dt(&fem_chl, 0);

    LOG_INF("SKY66112 set to transmit low-power mode (Mode 3)");

    gpio_pin_set_dt(&fem_ant_sel, 0);
    LOG_INF("SKY66112 antenna port 1 selected");

    return 0;
}

/* Register init function to run in PRE_KERNEL_1 phase */
SYS_INIT(_sky66112_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

/**
 * @brief Set SKY66112 to Transmit Low-Power Mode (Mode 3)
 *
 * Mode 3 settings (Vcc1 = 1.8 V, Vcc2 = 3.0 V, Vdd = 3.0 V, TA = +25 °C):
 *  CSD  = 1 (enable chip)
 *  CPS  = 0 (bypass/low-power gain)
 *  CRX  = 0 (disable receive)
 *  CTX  = 1 (enable transmit)
 *  CHL  = 0 (low-power mode)
 */
int sky66112EnableLowPowerTx(void)
{
    /* Bring FEM out of shutdown */
    gpio_pin_set_dt(&fem_csd, 1);
    /* Select bypass/low-power path */
    gpio_pin_set_dt(&fem_cps, 0);
    /* Disable receive */
    gpio_pin_set_dt(&fem_crx, 0);
    /* Enable transmit */
    gpio_pin_set_dt(&fem_ctx, 1);
    /* Set low-power mode */
    gpio_pin_set_dt(&fem_chl, 0);

    LOG_INF("SKY66112 set to transmit low-power mode (Mode 3)");
    return 0;
}

/**
 * @brief Select Antenna 1
 *
 * ANT1 port enabled: ANT_SEL = 0 (Vcc1 = 1.8 V, Vcc2 = 3.0 V, Vdd = 3.0 V, TA = +25 °C)
 */
int sky66112SelectAntenna1(void)
{
    /* ANT_SEL low => ANT1
       High would select ANT2 */
    gpio_pin_set_dt(&fem_ant_sel, 0);
    LOG_INF("SKY66112 antenna port 1 selected");
    return 0;
}
