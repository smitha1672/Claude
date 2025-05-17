/**
 * @file: pm_device_i2c.c
 * @brief I2C device power management implementation
 *
 * This file provides power management implementation for I2C bus devices
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pm_device_i2c, CONFIG_PM_DEVICE_I2C_LOG_LEVEL);

/**
 * @brief Structure for I2C PM device context
 *
 * Stores necessary context for I2C bus power management
 */
struct pm_device_i2c_context {
    /* Reference counter for active devices using the bus */
    atomic_t device_count;

    /* Lock for synchronizing access during power transitions */
    struct k_mutex lock;

    /* Current power state */
    enum pm_device_state current_state;

    /* Flag indicating if bus needs recovery after deep sleep */
    bool needs_recovery;
};

/* Forward declarations for internal functions */
static int pm_device_i2c_action(const struct device *dev,
                              enum pm_device_action action);

/**
 * @brief Define a power-managed I2C device for an instance
 */
#define PM_DEVICE_I2C_DEFINE(inst)                                \
    static struct pm_device_i2c_context pm_device_i2c_ctx_##inst = {  \
        .device_count = ATOMIC_INIT(0),                           \
        .current_state = PM_DEVICE_STATE_ACTIVE,                  \
        .needs_recovery = false,                                  \
    };                                                            \
                                                                  \
    PM_DEVICE_DT_INST_DEFINE(inst, pm_device_i2c_action);         \
                                                                  \
    static int pm_device_i2c_init_##inst(const struct device *dev) \
    {                                                             \
        struct pm_device_i2c_context *ctx = dev->data;            \
        k_mutex_init(&ctx->lock);                                 \
        return 0;                                                 \
    }                                                             \
                                                                  \
    DEVICE_DT_INST_DEFINE(inst,                                   \
                        pm_device_i2c_init_##inst,               \
                        NULL,                                     \
                        &pm_device_i2c_ctx_##inst,                \
                        NULL, POST_KERNEL,                        \
                        CONFIG_PM_DEVICE_I2C_INIT_PRIORITY,      \
                        NULL);

/**
 * @brief Acquire I2C bus for a device
 *
 * Increments the reference counter and ensures the bus is powered on.
 *
 * @param dev The I2C device to acquire
 * @return 0 on success, negative errno on failure
 */
static int _pm_device_i2c_acquire(const struct device *dev)
{
    struct pm_device_i2c_context *ctx = dev->data;
    int ret = 0;

    /* Increment the device counter */
    atomic_inc(&ctx->device_count);

    /* Make sure device is active if this is the first user */
    if (ctx->current_state != PM_DEVICE_STATE_ACTIVE) {
        ret = pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME);
    }

    /* Rollback reference count if activation failed */
    if (ret != 0) {
        atomic_dec(&ctx->device_count);
    }

    return ret;
}

/**
 * @brief Release I2C bus after a device is done using it
 *
 * Decrements the reference counter. The bus may be suspended
 * when the last user releases it, depending on power policy.
 *
 * @param dev The I2C device to release
 * @return 0 on success, negative errno on failure
 */
static int _pm_device_i2c_release(const struct device *dev)
{
    struct pm_device_i2c_context *ctx = dev->data;
    int ret = 0;

    /* Ensure counter doesn't go below zero */
    if (atomic_get(&ctx->device_count) == 0) {
        LOG_WRN("Device count already zero");
        return -EINVAL;
    }

    /* Decrement counter */
    atomic_dec(&ctx->device_count);

    /* If auto-suspend is enabled and no active users, schedule suspend */
    if (atomic_get(&ctx->device_count) == 0 && IS_ENABLED(CONFIG_PM_DEVICE_I2C_AUTO_SUSPEND)) {
        /* Use device runtime PM to schedule suspend */
        ret = pm_device_runtime_put(dev);
    }

    return ret;
}

/**
 * @brief Handler for recovery after deep sleep
 *
 * Recover I2C bus state after deep sleep cycles where the peripheral
 * might need reinitialization.
 *
 * @param dev The I2C device to recover
 * @return 0 on success, negative errno on failure
 */
static int pm_device_i2c_recover(const struct device *dev)
{
    const struct device *i2c_dev = dev;
    int ret = 0;

    if (IS_ENABLED(CONFIG_I2C_RECOVERY)) {
        LOG_DBG("Recovering I2C bus after deep sleep");
        ret = i2c_recover_bus(i2c_dev);
        if (ret != 0) {
            LOG_ERR("Failed to recover I2C bus: %d", ret);
        }
    }

    return ret;
}

/**
 * @brief I2C device power management action handler
 *
 * Implements the power management state machine for I2C devices
 *
 * @param dev The I2C device to perform action on
 * @param action The PM action to perform
 * @return 0 on success, negative errno on failure
 */
static int pm_device_i2c_action(const struct device *dev,
                              enum pm_device_action action)
{
    struct pm_device_i2c_context *ctx = dev->data;
    int ret = 0;

    k_mutex_lock(&ctx->lock, K_FOREVER);

    LOG_DBG("I2C PM action %d, current state %d", action, ctx->current_state);

    switch (action) {
    case PM_DEVICE_ACTION_SUSPEND:
        /* Only allow suspend if no active users */
        if (atomic_get(&ctx->device_count) > 0) {
            LOG_WRN("Cannot suspend, %d active users",
                   atomic_get(&ctx->device_count));
            ret = -EBUSY;
            break;
        }

        /* Transition to suspended state */
        ctx->current_state = PM_DEVICE_STATE_SUSPENDED;
        break;

    case PM_DEVICE_ACTION_RESUME:
        /* Perform recovery if needed */
        if (ctx->needs_recovery) {
            ret = pm_device_i2c_recover(dev);
            ctx->needs_recovery = false;
        }

        /* Transition to active state */
        ctx->current_state = PM_DEVICE_STATE_ACTIVE;
        break;

    case PM_DEVICE_ACTION_TURN_OFF:
        /* Only allow power off if no active users */
        if (atomic_get(&ctx->device_count) > 0) {
            LOG_WRN("Cannot turn off, %d active users",
                   atomic_get(&ctx->device_count));
            ret = -EBUSY;
            break;
        }

        /* Flag that recovery will be needed on next resume */
        ctx->needs_recovery = true;

        /* Transition to off state */
        ctx->current_state = PM_DEVICE_STATE_OFF;
        break;

    case PM_DEVICE_ACTION_TURN_ON:
        /* Always need recovery when turning back on */
        ret = pm_device_i2c_recover(dev);
        ctx->needs_recovery = false;

        /* Transition to suspended state - will need explicit resume */
        ctx->current_state = PM_DEVICE_STATE_SUSPENDED;
        break;

    default:
        LOG_ERR("Unsupported PM action %d", action);
        ret = -ENOTSUP;
    }

    k_mutex_unlock(&ctx->lock);
    return ret;
}

/*
 * I2C operation wrapper functions that include power management
 */

int pm_i2c_configure(const struct device *dev, uint32_t dev_config)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Perform actual configuration */
    ret = i2c_configure(dev, dev_config);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

int pm_i2c_transfer(const struct device *dev, struct i2c_msg *msgs,
                   uint8_t num_msgs, uint16_t addr)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Perform actual transfer */
    ret = i2c_transfer(dev, msgs, num_msgs, addr);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

int pm_i2c_recover_bus(const struct device *dev)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Perform actual bus recovery */
    ret = i2c_recover_bus(dev);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

int pm_i2c_get_config(const struct device *dev, uint32_t *dev_config)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Get current configuration */
    ret = i2c_get_config(dev, dev_config);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

int pm_i2c_slave_register(const struct device *dev,
                         struct i2c_slave_config *config)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Register slave device */
    ret = i2c_slave_register(dev, config);

    /* Don't release if successfully registered as slave */
    if (ret < 0) {
        _pm_device_i2c_release(dev);
    }

    return ret;
}

int pm_i2c_slave_unregister(const struct device *dev,
                           struct i2c_slave_config *config)
{
    int ret;

    /* Unregister slave device */
    ret = i2c_slave_unregister(dev, config);

    /* Release after unregistering */
    if (ret == 0) {
        _pm_device_i2c_release(dev);
    }

    return ret;
}

/*
 * Helper functions for reading/writing registers
 */

int pm_i2c_reg_read_byte(const struct device *dev, uint16_t dev_addr,
                        uint8_t reg_addr, uint8_t *value)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Perform register read */
    ret = i2c_reg_read_byte(dev, dev_addr, reg_addr, value);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

int pm_i2c_reg_write_byte(const struct device *dev, uint16_t dev_addr,
                         uint8_t reg_addr, uint8_t value)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Perform register write */
    ret = i2c_reg_write_byte(dev, dev_addr, reg_addr, value);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

int pm_i2c_reg_update_byte(const struct device *dev, uint16_t dev_addr,
                          uint8_t reg_addr, uint8_t mask, uint8_t value)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(dev);
    if (ret < 0) {
        return ret;
    }

    /* Perform register update */
    ret = i2c_reg_update_byte(dev, dev_addr, reg_addr, mask, value);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(dev);

    return ret;
}

/*
 * Devicetree-based helper functions
 */

int pm_i2c_burst_read_dt(const struct i2c_dt_spec *spec,
                        uint8_t start_addr, uint8_t *buf, uint32_t num_bytes)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform burst read */
    ret = i2c_burst_read_dt(spec, start_addr, buf, num_bytes);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_burst_write_dt(const struct i2c_dt_spec *spec,
                         uint8_t start_addr, const uint8_t *buf, uint32_t num_bytes)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform burst write */
    ret = i2c_burst_write_dt(spec, start_addr, buf, num_bytes);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_reg_read_byte_dt(const struct i2c_dt_spec *spec,
                           uint8_t reg_addr, uint8_t *value)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform register read */
    ret = i2c_reg_read_byte_dt(spec, reg_addr, value);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_reg_write_byte_dt(const struct i2c_dt_spec *spec,
                            uint8_t reg_addr, uint8_t value)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform register write */
    ret = i2c_reg_write_byte_dt(spec, reg_addr, value);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_reg_update_byte_dt(const struct i2c_dt_spec *spec,
                             uint8_t reg_addr, uint8_t mask, uint8_t value)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform register update */
    ret = i2c_reg_update_byte_dt(spec, reg_addr, mask, value);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_write_dt(const struct i2c_dt_spec *spec,
                   const uint8_t *buf, uint32_t num_bytes)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform write operation */
    ret = i2c_write_dt(spec, buf, num_bytes);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_read_dt(const struct i2c_dt_spec *spec,
                  uint8_t *buf, uint32_t num_bytes)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform read operation */
    ret = i2c_read_dt(spec, buf, num_bytes);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_transfer_dt(const struct i2c_dt_spec *spec,
                      struct i2c_msg *msgs, uint8_t num_msgs)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform transfer */
    ret = i2c_transfer_dt(spec, msgs, num_msgs);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

int pm_i2c_write_read_dt(const struct i2c_dt_spec *spec,
                        const void *write_buf, size_t num_write,
                        void *read_buf, size_t num_read)
{
    int ret;

    /* Acquire I2C bus before operation */
    ret = _pm_device_i2c_acquire(spec->bus);
    if (ret < 0) {
        return ret;
    }

    /* Perform write-read operation */
    ret = i2c_write_read_dt(spec, write_buf, num_write, read_buf, num_read);

    /* Release I2C bus after operation */
    _pm_device_i2c_release(spec->bus);

    return ret;
}

/**
 * @brief Check if I2C bus is currently in use
 *
 * @param dev The I2C device to check
 * @return true if bus is in use, false otherwise
 */
bool pm_device_i2c_is_busy(const struct device *dev)
{
    struct pm_device_i2c_context *ctx = dev->data;
    return atomic_get(&ctx->device_count) > 0;
}

/* Create the device instance for each enabled I2C instance */
#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_twi)
PM_DEVICE_I2C_DEFINE(0);
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_twim)
PM_DEVICE_I2C_DEFINE(0);
#endif
