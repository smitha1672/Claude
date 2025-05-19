/**
 * @file: pm_device_i2c.h
 * @brief I2C device power management interface
 *
 * This file provides power management interface for I2C bus devices
 *
 */

#ifndef _PM_DEVICE_MODULES_PM_DEVICE_I2C_H_
#define _PM_DEVICE_MODULES_PM_DEVICE_I2C_H_

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure an I2C device with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param dev_config I2C configuration
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_configure(const struct device *dev, uint32_t dev_config);

/**
 * @brief Transfer messages through I2C bus with power management
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param msgs Array of messages to transfer
 * @param num_msgs Number of messages to transfer
 * @param addr Address of the I2C target device
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_transfer(const struct device *dev, struct i2c_msg *msgs,
                   uint8_t num_msgs, uint16_t addr);

/**
 * @brief Recover I2C bus with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_recover_bus(const struct device *dev);

/**
 * @brief Get I2C device configuration with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param dev_config Pointer to receive the I2C configuration
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_get_config(const struct device *dev, uint32_t *dev_config);

/**
 * @brief Register an I2C slave service with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param config I2C slave config
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_slave_register(const struct device *dev,
                         struct i2c_slave_config *config);

/**
 * @brief Unregister an I2C slave service with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param config I2C slave config
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_slave_unregister(const struct device *dev,
                           struct i2c_slave_config *config);

/**
 * @brief Read a byte from an I2C register with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param dev_addr Address of the I2C device
 * @param reg_addr Address of the register to read
 * @param value Pointer to receive the register value
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_reg_read_byte(const struct device *dev, uint16_t dev_addr,
                        uint8_t reg_addr, uint8_t *value);

/**
 * @brief Write a byte to an I2C register with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param dev_addr Address of the I2C device
 * @param reg_addr Address of the register to write
 * @param value Value to write to the register
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_reg_write_byte(const struct device *dev, uint16_t dev_addr,
                         uint8_t reg_addr, uint8_t value);

/**
 * @brief Update a byte in an I2C register with power management support
 *
 * @param dev Pointer to the device structure for the driver instance
 * @param dev_addr Address of the I2C device
 * @param reg_addr Address of the register to update
 * @param mask Bitmask of bits to update
 * @param value Value to write to the register
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_reg_update_byte(const struct device *dev, uint16_t dev_addr,
                          uint8_t reg_addr, uint8_t mask, uint8_t value);

/**
 * @brief Perform a burst read from an I2C device with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param start_addr Starting address to read from
 * @param buf Buffer to read data into
 * @param num_bytes Number of bytes to read
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_burst_read_dt(const struct i2c_dt_spec *spec,
                        uint8_t start_addr, uint8_t *buf, uint32_t num_bytes);

/**
 * @brief Perform a burst write to an I2C device with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param start_addr Starting address to write to
 * @param buf Buffer containing data to write
 * @param num_bytes Number of bytes to write
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_burst_write_dt(const struct i2c_dt_spec *spec,
                         uint8_t start_addr, const uint8_t *buf, uint32_t num_bytes);

/**
 * @brief Read a byte from an I2C register with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param reg_addr Address of the register to read
 * @param value Pointer to receive the register value
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_reg_read_byte_dt(const struct i2c_dt_spec *spec,
                           uint8_t reg_addr, uint8_t *value);

/**
 * @brief Write a byte to an I2C register with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param reg_addr Address of the register to write
 * @param value Value to write to the register
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_reg_write_byte_dt(const struct i2c_dt_spec *spec,
                            uint8_t reg_addr, uint8_t value);

/**
 * @brief Update a byte in an I2C register with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param reg_addr Address of the register to update
 * @param mask Bitmask of bits to update
 * @param value Value to write to the register
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_reg_update_byte_dt(const struct i2c_dt_spec *spec,
                             uint8_t reg_addr, uint8_t mask, uint8_t value);

/**
 * @brief Write data to an I2C device with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param buf Buffer containing data to write
 * @param num_bytes Number of bytes to write
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_write_dt(const struct i2c_dt_spec *spec,
                   const uint8_t *buf, uint32_t num_bytes);

/**
 * @brief Read data from an I2C device with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param buf Buffer to read data into
 * @param num_bytes Number of bytes to read
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_read_dt(const struct i2c_dt_spec *spec,
                  uint8_t *buf, uint32_t num_bytes);

/**
 * @brief Transfer messages through I2C bus with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param msgs Array of messages to transfer
 * @param num_msgs Number of messages to transfer
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_transfer_dt(const struct i2c_dt_spec *spec,
                      struct i2c_msg *msgs, uint8_t num_msgs);

/**
 * @brief Write to and read from an I2C device with devicetree node support
 *
 * @param spec I2C specification from devicetree
 * @param write_buf Buffer containing data to write
 * @param num_write Number of bytes to write
 * @param read_buf Buffer to read data into
 * @param num_read Number of bytes to read
 *
 * @retval 0 If successful
 * @retval -errno Negative errno code on failure
 */
int pm_i2c_write_read_dt(const struct i2c_dt_spec *spec,
                        const void *write_buf, size_t num_write,
                        void *read_buf, size_t num_read);

/**
 * @brief Check if I2C bus is currently in use
 *
 * @param dev The I2C device to check
 * @return true if bus is in use, false otherwise
 */
bool pm_device_i2c_is_busy(const struct device *dev);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_PM_DEVICE_MODULES_PM_DEVICE_I2C_H_ */
