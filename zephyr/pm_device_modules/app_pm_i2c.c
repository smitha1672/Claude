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
