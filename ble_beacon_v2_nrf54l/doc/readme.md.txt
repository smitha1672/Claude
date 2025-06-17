# Beacon V2 Information

**Company**: Motive  
**Project**: Beacon V2 (BC2)  
**Board**: beacon_v2_nrf54l15/nrf54l10(Planned)  
**Microcontroller**: nRF54L15/nRF54L10  
**SDK**: Nordic nRF Connect SDK v3.0.0  
**RTOS**: Nordic Zephyr v4.0.99-ncs1  

## Overview

The BC2 firmware is an embedded system designed for BLE beacon devices using Nordic's nRF54L15/nRF54L10 chipset with Zephyr RTOS. The firmware provides comprehensive hardware validation, RF testing capabilities, and production-ready beacon functionality.

### Hardware Specifications

- **Main Controller**: nRF54L15-QFAA or nRF54L10-QFAA (ARM Cortex-M33)
- **Flash**: 1.5 MB (nRF54L15) / 1.0 MB (nRF54L10)
- **RAM**: 256 KB (nRF54L15) / 128 KB (nRF54L10)  
- **Wireless**: Bluetooth Low Energy (BLE) 5.4
- **RTC**: PCF85063A
- **FEM**: SKY66112 (RF Front-End Module)
- **PMIC**: nPM2100 (Power Management IC)

#### Chip Variant Support
| Feature | nRF54L15 | nRF54L10 |
|---------|----------|----------|
| Flash Memory | 1.5 MB | 1.0 MB |
| RAM Memory | 256 KB | 128 KB |
| Pin Compatibility | Yes | Yes (Same pinout) |
| Firmware Support | Yes | Planned |

---

## Firmware Architecture

### Project Structure

```
./ble_beacon_v2_nrf54l/
├── APP_BLT                      # Board-Level Test application
│   ├── src
│   └── sysbuild
├── APP_RF_DirectTestMode        # RF Direct Test Mode application
├── boards                       # Board support files
│   └── motive
│       ├── beacon_v2            # Main board definition
│       └── beacon_v2_overlay    # Board configuration overlays
├── drivers                      # Out-of-tree Hardware drivers
│   ├── fem                      # Front-End Module driver
│   ├── gpio                     # GPIO driver
│   ├── mfd                      # Multi-Function Device driver
│   ├── regulator                # Power regulation driver
│   ├── rtc                      # Real-Time Clock driver
│   ├── sensor                   # Sensor drivers
│   │   └── npm2100_vbat         # nPM2100 battery voltage sensor
│   └── watchdog                 # Watchdog timer driver
├── dts
│   └── bindings                 # Device tree bindings
│       ├── fem
│       ├── gpio
│       ├── mfd
│       ├── regulator
│       ├── rtc
│       ├── sensor
│       └── watchdog
├── include                      # Header files
│   ├── fem
│   └── shared                   # Shared module headers
│       ├── bt_modules
│       ├── ext_rtc_module
│       ├── pm_device_modules
│       ├── pmic_modules
│       └── rram
├── overlay_conf                 # Configuration overlays
├── scripts                      # Provisioning device tool (planned)
├── shared                       # Shared modules/libraries
│   ├── bt_modules               # Bluetooth modules
│   ├── ext_rtc_module           # External RTC module
│   ├── pm_device_modules        # Power management device modules
│   ├── pmic_modules             # PMIC modules
│   ├── rram                     # RRAM module (planned)
│   └── shell                    # Shell command modules
```

### What Each Part Does

#### Applications
| Component | Purpose |
|-----------|---------|
| **APP_BLT** | Board-Level Test application with shell commands for peripheral validation |
| **APP_RF_DirectTestMode** | RF certification testing |

#### Out-of-tree Hardware Drivers
| Driver | Hardware | Function |
|--------|----------|----------|
| **drivers/fem** | SKY66112 | RF front-end control, TX/RX switching, antenna diversity |
| **drivers/gpio** | Button, nPM2100 GPIO | Button input handling and nPM2100 GPIO control (planned) |
| **drivers/mfd** | nPM2100 | nPM2100 multi-function device management |
| **drivers/regulator** | nPM2100 | Power regulation and voltage control |
| **drivers/rtc** | PCF85063A | Real-time clock operations |
| **drivers/sensor/npm2100_vbat** | nPM2100 | Battery voltage monitoring |
| **drivers/watchdog** | nPM2100 | nPM2100 watchdog timer control |

#### Device Tree Bindings
| Binding | Purpose |
|---------|---------|
| **dts/bindings/** | Device tree descriptions for all custom drivers |

#### Board Support
| Directory | Contents |
|-----------|----------|
| **boards/motive/beacon_v2** | Main board definition and pin assignments |
| **boards/motive/beacon_v2_overlay** | Board configuration overlays for different variants |

#### Shared Modules
| Module | Purpose |
|--------|---------|
| **shared/bt_modules** | Bluetooth central/peripheral roles (planned), UART interface, beacon examples, FOTA |
| **shared/ext_rtc_module** | External RTC management and time synchronization |
| **shared/pm_device_modules** | Power management device console interface |
| **shared/pmic_modules** | PMIC control and battery management |
| **shared/rram** | RRAM module functionality (planned) |
| **shared/shell** | JSON format output for manufacturing machine and shell functionality for board level test |

#### Support Files
| Directory | Contents |
|-----------|----------|
| **include/** | Public header files and API definitions |
| **overlay_conf** | Build configuration overlays for different variants |
| **scripts** | Provisioning device tool (planned) |

---

## Build Environment Setup
