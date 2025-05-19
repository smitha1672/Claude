/**
 * Copyright (c) 2025, Motive
 *
 * SPDX-License-Identifier: Proprietary
 */

/**
 * @file sky66112.h
 * @brief Driver for Skyworks SKY66112-11 RF front-end module
 *
 * This module provides control of the SKY66112-11 FEM for the Beacon V2 project
 */

#ifndef SKY66112_H_
#define SKY66112_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

/**
 * @brief SKY66112 operating modes
 */
typedef enum {
    SKY66112_MODE_SLEEP,     /**< Sleep mode (low power) */
    SKY66112_MODE_RX,        /**< Receive mode */
    SKY66112_MODE_TX_LOW,    /**< Transmit mode with low power */
    SKY66112_MODE_TX_HIGH,   /**< Transmit mode with high power */
} Sky66112Mode;

/**
 * @brief SKY66112 antenna selection
 */
typedef enum {
    SKY66112_ANT1,           /**< Antenna 1 */
    SKY66112_ANT2            /**< Antenna 2 */
} Sky66112Antenna;

/**
 * @brief SKY66112 device configuration structure
 */
struct sky66112_config {
    /** GPIO for chip shutdown control (CSD) */
    struct gpio_dt_spec csd_gpio;
    
    /** GPIO for PA/LNA vs bypass mode selection (CPS) */
    struct gpio_dt_spec cps_gpio;
    
    /** GPIO for receive enable (CRX) */
    struct gpio_dt_spec crx_gpio;
    
    /** GPIO for transmit enable (CTX) */
    struct gpio_dt_spec ctx_gpio;
    
    /** GPIO for power mode selection (CHL) */
    struct gpio_dt_spec chl_gpio;
    
    /** GPIO for antenna selection (ANT_SEL) */
    struct gpio_dt_spec ant_sel_gpio;
};

/**
 * @brief SKY66112 device data structure
 */
struct sky66112_data {
    /** Current operating mode */
    Sky66112Mode current_mode;
    
    /** Currently selected antenna */
    Sky66112Antenna current_antenna;
};

/**
 * @brief Initialize the SKY66112 device
 *
 * @param dev Device pointer to the SKY66112 device
 * @return 0 if successful, negative errno code otherwise
 */
int sky66112Init(const struct device *dev);

/**
 * @brief Set the SKY66112 operating mode
 *
 * @param dev Device pointer to the SKY66112 device
 * @param mode Desired operating mode
 * @return 0 if successful, negative errno code otherwise
 */
int sky66112SetMode(const struct device *dev, Sky66112Mode mode);

/**
 * @brief Get the current SKY66112 operating mode
 *
 * @param dev Device pointer to the SKY66112 device
 * @return Current operating mode
 */
Sky66112Mode sky66112GetMode(const struct device *dev);

/**
 * @brief Select antenna to use
 *
 * @param dev Device pointer to the SKY66112 device
 * @param antenna Antenna to select
 * @return 0 if successful, negative errno code otherwise
 */
int sky66112SelectAntenna(const struct device *dev, Sky66112Antenna antenna);

/**
 * @brief Get the currently selected antenna
 *
 * @param dev Device pointer to the SKY66112 device
 * @return Currently selected antenna
 */
Sky66112Antenna sky66112GetAntenna(const struct device *dev);

/**
 * @brief Get the TX gain for the current mode
 *
 * @param dev Device pointer to the SKY66112 device
 * @return TX gain in dB, or 0 if not in TX mode
 */
uint8_t sky66112GetTxGain(const struct device *dev);

/**
 * @brief Get the RX gain
 *
 * @param dev Device pointer to the SKY66112 device
 * @return RX gain in dB, or 0 if not in RX mode
 */
uint8_t sky66112GetRxGain(const struct device *dev);

/**
 * @brief API implementation for Zephyr PM device control
 */
int sky66112PmControl(const struct device *dev, enum pm_device_action action);

#endif /* SKY66112_H_ */
