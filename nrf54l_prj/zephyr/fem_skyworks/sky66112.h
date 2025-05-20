/**
 * @file sky66112.h
 * @brief Driver header for the SKY66112 FEM on nRF54L15 platform.
 *
 * Provides APIs to control individual pins of the SKY66112 FEM
 * as well as setting it into various operating and antenna modes.
 *
 * Copyright (c) 2025 Motive
 */

#ifndef SKY66112_H_
#define SKY66112_H_

#include <stdint.h>

/**
 * @brief Set SKY66112 CSD pin state
 * 
 * Controls Chip Shutdown pin:
 * 0 = Shutdown mode (disabled)
 * 1 = Active mode (enabled)
 *
 * @param state The state to set (0 or 1)
 * @return 0 on success, negative errno on failure
 */
int sky66112SetCsd(uint8_t state);

/**
 * @brief Set SKY66112 CPS pin state
 * 
 * Controls Power Select pin:
 * 0 = Bypass/low-power gain path
 * 1 = LNA path (receive mode)
 *
 * @param state The state to set (0 or 1)
 * @return 0 on success, negative errno on failure
 */
int sky66112SetCps(uint8_t state);

/**
 * @brief Set SKY66112 CRX pin state
 * 
 * Controls Receive Enable pin:
 * 0 = Receive disabled
 * 1 = Receive enabled
 *
 * @param state The state to set (0 or 1)
 * @return 0 on success, negative errno on failure
 */
int sky66112SetCrx(uint8_t state);

/**
 * @brief Set SKY66112 CTX pin state
 * 
 * Controls Transmit Enable pin:
 * 0 = Transmit disabled
 * 1 = Transmit enabled
 *
 * @param state The state to set (0 or 1)
 * @return 0 on success, negative errno on failure
 */
int sky66112SetCtx(uint8_t state);

/**
 * @brief Set SKY66112 CHL pin state
 * 
 * Controls High/Low Power Select pin:
 * 0 = Low-power mode
 * 1 = High-power mode
 *
 * @param state The state to set (0 or 1)
 * @return 0 on success, negative errno on failure
 */
int sky66112SetChl(uint8_t state);

/**
 * @brief Set SKY66112 ANT_SEL pin state
 * 
 * Controls Antenna Select pin:
 * 0 = Antenna 1 selected
 * 1 = Antenna 2 selected
 *
 * @param state The state to set (0 or 1)
 * @return 0 on success, negative errno on failure
 */
int sky66112SetAntSel(uint8_t state);

/**
 * @brief Set SKY66112 to Transmit Low-Power Mode (Mode 3)
 *
 * Mode 3 settings (Vcc1 = 1.8 V, Vcc2 = 3.0 V, Vdd = 3.0 V, TA = +25 °C):
 *  CSD  = 1 (enable chip)
 *  CPS  = 0 (bypass/low-power gain)
 *  CRX  = 0 (disable receive)
 *  CTX  = 1 (enable transmit)
 *  CHL  = 0 (low-power mode)
 *
 * @return 0 on success
 */
int sky66112EnableLowPowerTx(void);

/**
 * @brief Select Antenna 1
 *
 * ANT1 port enabled: ANT_SEL = 0 (Vcc1 = 1.8 V, Vcc2 = 3.0 V, Vdd = 3.0 V, TA = +25 °C)
 *
 * @return 0 on success
 */
int sky66112SelectAntenna1(void);

/**
 * @brief FEM operating modes
 */
#define SKY66112_MODE_SHUTDOWN    0  /* Device in shutdown (disabled) */
#define SKY66112_MODE_RECEIVE     1  /* Receive mode */
#define SKY66112_MODE_TX_HIGHPWR  2  /* Transmit high-power mode */
#define SKY66112_MODE_TX_LOWPWR   3  /* Transmit low-power mode */
#define SKY66112_MODE_BYPASS      4  /* Bypass mode (direct path) */

#endif /* SKY66112_H_ */
