# BLE Beacon V2 Firmware

This project contains the firmware for the `beacon_v2_nrf54l15` platform, built on Nordic's nRF54L15 using the Nordic Connect SDK (NCS) and Zephyr RTOS.

## 📁 Project Structure

```
ble_beacon_v2_nrf54l/
├── applications/               # Main application and modules
├── app_zephyr_smp_svr/        # FOTA (SMP server) support
├── boards/motive/             # Board definitions and overlays
├── drivers/led/               # Custom LED driver (IS31FL3197)
├── dts/bindings/led/          # Devicetree bindings
├── include/shared/            # Shared header files
├── scripts/                   # Utility scripts
├── shared/                    # Bluetooth and shell modules
└── zephyr/                    # Zephyr RTOS modules
```

## 🛠️ Prerequisites

These tools are pre-installed in the provided Docker image:

* Nordic SDK: NCS v3.0.0
* Zephyr: v4.0.99-ncs1
* Python 3.8+
* West (Zephyr build tool)
* nRF Command Line Tools (`nrfjprog`)

## ⚙️ Setup

```bash
west init -l ble_beacon_v2_nrf54l
west update
source zephyr/zephyr-env.sh
```

## 🧱 Build Instructions

```bash
# For application build
west build -p always -b beacon_v2_nrf54l15_cpuapp applications -- -DCONFIG_DEBUG=y

# For FOTA SMP server build
west build -p always -b beacon_v2_nrf54l15_cpuapp app_zephyr_smp_svr
```

## 🚀 Flashing Firmware

```bash
nrfjprog --eraseall -f nrf54l15
nrfjprog --program build/zephyr/zephyr.hex --chiperase --verify --reset
```

## ✅ Deployment Checklist

* [ ] Confirm correct overlay for board stage (proto/dev/etc.)
* [ ] Ensure `boards/motive/beacon_v2` matches hardware
* [ ] LED driver `is31fl3197` is configured
* [ ] BLE advertising power and interval settings are validated
* [ ] Watchdog functionality is tested under `src/app/watchdog`
* [ ] FOTA (SMP server) functionality verified

## 📎 Notes

* Use `prj.conf` for production; `prj_debug.conf` during development.
* Overlay files are located in `beacon_v2_overlay`.
* Shell features are implemented under `shared/shell`.

---

For detailed internal documentation, refer to:
`boards/motive/beacon_v2/doc/`
