/**
 * @brief:     PMIC NPM2100 module
 * @file:      pmic_regulator.c
 * @date:      2025-05-25
 * @copyright: Copyright (c) 2025 Motive Technologies, Inc. All rights reserved.
 */

#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/mfd/npm2100.h>
#include <zephyr/drivers/regulator.h>

LOG_MODULE_REGISTER(pmic_regulator, LOG_LEVEL_DBG);

static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm2100_pmic));
static const struct device *regulators = DEVICE_DT_GET(DT_NODELABEL(npm2100_regulators));

__weak void event_callback(const struct device *dev, struct gpio_callback *cb, uint32_t events)
{
	static int64_t press_t;

	if (events & BIT(NPM2100_EVENT_SYS_SHIPHOLD_FALL)) {
		press_t = k_uptime_get();
	}

	if (events & BIT(NPM2100_EVENT_SYS_SHIPHOLD_RISE)) {
		int64_t delta_t = k_uptime_get() - press_t;

		if (delta_t < PRESS_SHORT_MS) {
			LOG_DBG("Short press");
			flash_time_ms = FLASH_FAST_MS;
		} else if (delta_t < PRESS_MEDIUM_MS) {
			LOG_DBG("Medium press");
			flash_time_ms = FLASH_SLOW_MS;
		} else {
			shipmode = true;
		}
	}
}

static bool configure_events(void)
{
	int ret;
	static struct gpio_callback event_cb;

	if (!device_is_ready(pmic)) {
		LOG_DBG("Error: PMIC device not ready.");
		return false;
	}

	if (!device_is_ready(regulators)) {
		LOG_DBG("Error: Regulator device not ready.");
		return false;
	}

	gpio_init_callback(&event_cb, event_callback,
			   BIT(NPM2100_EVENT_SYS_SHIPHOLD_FALL) |
				   BIT(NPM2100_EVENT_SYS_SHIPHOLD_RISE));

	ret = mfd_npm2100_add_callback(pmic, &event_cb);

	if (ret < 0) {
		LOG_DBG("Error: failed to add a PMIC event callback.");
		return false;
	}

	return true;
}

int pmic_regulator_ship_mode(void)
{
    return 0;
}
