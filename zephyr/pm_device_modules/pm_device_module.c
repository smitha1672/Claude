/**
 * @brief     Power management module
 * @file      pm_device_module.c
 * @date      2025-05-09
 * @copyright Copyright (c) 2025 Motive Technologies, Inc. All rights reserved.
  */

#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pm_device_module, LOG_LEVEL_NONE);

static const struct device *i2c0Dev = DEVICE_DT_GET(DT_NODELABEL(i2c21));
static const struct device *uart0Dev = DEVICE_DT_GET(DT_NODELABEL(uart30));

/**
 * @brief Initialize the power-managed peripherals (I2C0, UART0).
 *
 * Resumes peripherals if ready.
 *
 * @param dev Unused device pointer.
 * @return 0 on success, or negative errno code on failure.
 */
int pm_device_module_init(void)
{
    int ret = 0;

    if (!device_is_ready(i2c0Dev)) {
        LOG_ERR("I2C0 device not ready");
        ret = -ENODEV;
    } else {
        int resumeRet = pm_device_action_run(i2c0Dev, PM_DEVICE_ACTION_SUSPEND);
        if (resumeRet && resumeRet != -EALREADY) {
            LOG_ERR("Failed to resume I2C0 (err=%d)", resumeRet);
            ret = resumeRet;
        }
    }

    if (!device_is_ready(uart0Dev)) {
        LOG_ERR("UART0 device not ready");
        ret = -ENODEV;
    } else {
        int resumeRet = pm_device_action_run(uart0Dev, PM_DEVICE_ACTION_RESUME);
        if (resumeRet && resumeRet != -EALREADY) {
            LOG_ERR("Failed to resume UART0 (err=%d)", resumeRet);
            if (ret == 0) {
                ret = resumeRet;
            }
        }
    }

    return ret;
}

/**
 * @brief Suspend I2C0 peripheral.
 *
 * @return 0 on success, -ENODEV if device is not ready.
 */
int pm_device_module_suspend_i2c0(void)
{
    int ret = 0;

    if (!device_is_ready(i2c0Dev)) {
        return -ENODEV;
    }

    ret = pm_device_action_run(i2c0Dev, PM_DEVICE_ACTION_SUSPEND);
    if (ret == -EALREADY) {
        return 0;
    }

    return ret;
}

/**
 * @brief Resume I2C0 peripheral.
 *
 * @return 0 on success, -ENODEV if device is not ready.
 */
int pm_device_module_resume_i2c0(void)
{
    int ret = 0;

    if (!device_is_ready(i2c0Dev)) {
        return -ENODEV;
    }

    ret = pm_device_action_run(i2c0Dev, PM_DEVICE_ACTION_RESUME);
    if (ret == -EALREADY) {
        return 0;
    }

    return ret;
}

/**
 * @brief Suspend UART0 peripheral.
 *
 * @return 0 on success, -ENODEV if device is not ready.
 */
int pm_device_module_suspend_uart0(void)
{
    int ret = 0;

    if (!device_is_ready(uart0Dev)) {
        return -ENODEV;
    }

    ret = pm_device_action_run(uart0Dev, PM_DEVICE_ACTION_SUSPEND);
    if (ret == -EALREADY) {
        return 0;
    }

    return ret;
}

/**
 * @brief Resume UART0 peripheral.
 *
 * @return 0 on success, -ENODEV if device is not ready.
 */
int pm_device_module_resume_uart0(void)
{
    int ret = 0;

    if (!device_is_ready(uart0Dev)) {
        return -ENODEV;
    }

    ret = pm_device_action_run(uart0Dev, PM_DEVICE_ACTION_RESUME);
    if (ret == -EALREADY) {
        return 0;
    }

    return ret;
}
