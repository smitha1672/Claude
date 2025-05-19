#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2c_example, CONFIG_APP_LOG_LEVEL);

/* Define the I2C device we'll be using */
#define I2C_DEV_NODE DT_NODELABEL(i2c0)

/* Function declaration */
int recoverI2cAfterSleep(void);

/**
 * @brief Application main entry point
 */
void main(void)
{
    /* Application initialization code... */
    LOG_INF("Application started");
    
    /* Simulate waking up from deep sleep */
    recoverI2cAfterSleep();
    
    /* Continue with normal operation */
    while (1) {
        k_sleep(K_SECONDS(1));
    }
}

int recoverI2cAfterSleep(void)
{
    const struct device *i2c_dev;
    int ret;
    
    /* Get the I2C device */
    i2c_dev = DEVICE_DT_GET(I2C_DEV_NODE);
    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C device not ready");
        return -ENODEV;
    }
    
    LOG_INF("Recovering I2C bus after sleep");
    
    /* Call the recovery function */
    ret = pm_device_i2c_recover_from_sleep(i2c_dev);
    if (ret != 0) {
        LOG_ERR("Failed to recover I2C bus: %d", ret);
        return ret;
    }
    
    LOG_INF("I2C bus recovered successfully");
    return 0;
}

int system_prepare_for_sleep(void)
{
    const struct device *i2c0_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
    
    /* Check if I2C is currently busy before allowing deep sleep */
    if (pm_device_i2c_is_busy(i2c0_dev)) {
        /* Either wait for I2C operations to complete or use lighter sleep mode */
        return -EBUSY;
    }
    
    /* Otherwise proceed with deep sleep preparations */
    return 0;
}


