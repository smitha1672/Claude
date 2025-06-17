# BLE Beacon V2 – Shared Source

This is the main application directory for the BLE Beacon V2 project based on the Nordic nRF54L15 SoC.

It integrates core BLE firmware, platform drivers, and optional shared modules for debugging and validation (e.g., shell utilities, power management, RTC, and clock testing).

---

## Shared Shell Modules Integration

Optional shell utilities are located under:

```
shared/shell/
```

These modules are designed to be conditionally compiled and included via Kconfig. They support features like clock control, power management, and diagnostics useful for firmware bring-up, validation, and production testing.

---

## Naming Convention

> ✅ All shared shell commands **must be registered with the prefix `board_`** using `SHELL_CMD_REGISTER`.

This ensures consistent naming and avoids shell command conflicts across modules.

### Example:

```c
SHELL_CMD_REGISTER(board_clk, &clock_test_cmds, "Board-level HFCLK test", NULL);
```

---

## How to Enable Shared Shell Modules

To enable any shared shell utility:

### 1. Enable Parent Configuration in `prj.conf`

```conf
CONFIG_SHARED_SHELL=y
```

This flag enables access to all sub-options in `shared/shell/Kconfig`.

---

### 2. Enable Specific Module(s)

For example, to enable the HFCLK test commands:

```conf
CONFIG_SHELL_CLOCK_TEST=y
```

Other modules (e.g., RTC, PM, FATP) can be enabled the same way, if available in `shared/shell/`.

---

## Required Kconfig Entry

In `application/Kconfig`, make sure to include:

```kconfig
rsource "shared/shell/Kconfig"
```

This makes the shared shell configurations available to the build system.

---

## Example: Enable `shell_clock_test.c`

### `prj.conf`

```conf
CONFIG_SHARED_SHELL=y
CONFIG_SHELL_CLOCK_TEST=y
```

---

## Shell Command Usage

After flashing the firmware with these configurations, the following commands are available:

```sh
board_clk start     # Starts the 32 MHz HFCLK
board_clk stop      # Stops and releases the HFCLK
```

---

## Summary

| File                        | Purpose                                |
|-----------------------------|----------------------------------------|
| `prj.conf`                  | Enable features with `CONFIG_` flags   |
| `Kconfig`                   | Import shared configs                  |
| `shared/shell/Kconfig`      | Define modular shell configs           |
| `shared/shell/CMakeLists.txt` | Add files conditionally by config     |

This modular setup ensures clean integration of debug/test tools while keeping them optional and configurable.
