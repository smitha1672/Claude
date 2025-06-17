# Beacon V2 Information

**Company**: Motive  
**Project**: Beacon V2 (BC2)  
**Board**: beacon_v2_nrf54l15  
**Microcontroller**: nRF54L15/nRF54L10  
**SDK**: Nordic nRF Connect SDK v3.0.0  
**RTOS**: Nordic Zephyr v4.0.99-ncs1  

## Overview

The BC2 firmware is an embedded system designed for IoT beacon devices using Nordic's nRF54L15/nRF54L10 chipset with Zephyr RTOS. The firmware provides comprehensive hardware validation, RF testing capabilities, and production-ready beacon functionality.

### Hardware Specifications

- **Main Controller**: nRF54L15-QFAA or nRF54L10-QFAA (ARM Cortex-M33)
- **Flash**: 1.5 MB (nRF54L15) / 1.0 MB (nRF54L10)
- **RAM**: 256 KB (nRF54L15) / 128 KB (nRF54L10)  
- **Wireless**: Bluetooth Low Energy (BLE) 5.4
- **RTC**: PCF85063A (I²C interface)
- **FEM**: SKY66112 (RF Front-End Module)
- **PMIC**: nPM2100 (Power Management IC)

#### Chip Variant Support
| Feature | nRF54L15 | nRF54L10 |
|---------|----------|----------|
| Flash Memory | 1.5 MB | 1.0 MB |
| RAM Memory | 256 KB | 128 KB |
| Pin Compatibility | ✓ | ✓ (Same pinout) |
| Firmware Support | ✓ | x (Planned) |

---

