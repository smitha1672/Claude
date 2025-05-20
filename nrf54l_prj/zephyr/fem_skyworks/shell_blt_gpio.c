/**
 * @file shell_blt_gpio.c
 * @brief Shell commands for Board Level Testing of SKY66112 FEM GPIO functionality
 *
 * Copyright (c) 2025 Motive
 */

#include <zephyr/shell/shell.h>
#include <zephyr/device.h>
#include <stdint.h>
#include <stdlib.h>
#include "sky66112.h"

/**
 * @brief Command handler for setting the FEM CSD pin
 */
static int cmd_fem_csd(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_error(shell, "Usage: %s <0|1>", argv[0]);
        return -EINVAL;
    }

    int state = atoi(argv[1]);
    if (state != 0 && state != 1) {
        shell_error(shell, "Invalid state. Use 0 or 1");
        return -EINVAL;
    }

    int ret = sky66112SetCsd(state);
    if (ret) {
        shell_error(shell, "Failed to set CSD pin: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 CSD pin set to %d", state);
    return 0;
}

/**
 * @brief Command handler for setting the FEM CPS pin
 */
static int cmd_fem_cps(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_error(shell, "Usage: %s <0|1>", argv[0]);
        return -EINVAL;
    }

    int state = atoi(argv[1]);
    if (state != 0 && state != 1) {
        shell_error(shell, "Invalid state. Use 0 or 1");
        return -EINVAL;
    }

    int ret = sky66112SetCps(state);
    if (ret) {
        shell_error(shell, "Failed to set CPS pin: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 CPS pin set to %d", state);
    return 0;
}

/**
 * @brief Command handler for setting the FEM CRX pin
 */
static int cmd_fem_crx(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_error(shell, "Usage: %s <0|1>", argv[0]);
        return -EINVAL;
    }

    int state = atoi(argv[1]);
    if (state != 0 && state != 1) {
        shell_error(shell, "Invalid state. Use 0 or 1");
        return -EINVAL;
    }

    int ret = sky66112SetCrx(state);
    if (ret) {
        shell_error(shell, "Failed to set CRX pin: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 CRX pin set to %d", state);
    return 0;
}

/**
 * @brief Command handler for setting the FEM CTX pin
 */
static int cmd_fem_ctx(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_error(shell, "Usage: %s <0|1>", argv[0]);
        return -EINVAL;
    }

    int state = atoi(argv[1]);
    if (state != 0 && state != 1) {
        shell_error(shell, "Invalid state. Use 0 or 1");
        return -EINVAL;
    }

    int ret = sky66112SetCtx(state);
    if (ret) {
        shell_error(shell, "Failed to set CTX pin: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 CTX pin set to %d", state);
    return 0;
}

/**
 * @brief Command handler for setting the FEM CHL pin
 */
static int cmd_fem_chl(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_error(shell, "Usage: %s <0|1>", argv[0]);
        return -EINVAL;
    }

    int state = atoi(argv[1]);
    if (state != 0 && state != 1) {
        shell_error(shell, "Invalid state. Use 0 or 1");
        return -EINVAL;
    }

    int ret = sky66112SetChl(state);
    if (ret) {
        shell_error(shell, "Failed to set CHL pin: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 CHL pin set to %d", state);
    return 0;
}

/**
 * @brief Command handler for setting the FEM ANT_SEL pin
 */
static int cmd_fem_ant_sel(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_error(shell, "Usage: %s <0|1>", argv[0]);
        return -EINVAL;
    }

    int state = atoi(argv[1]);
    if (state != 0 && state != 1) {
        shell_error(shell, "Invalid state. Use 0 or 1");
        return -EINVAL;
    }

    int ret = sky66112SetAntSel(state);
    if (ret) {
        shell_error(shell, "Failed to set ANT_SEL pin: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 ANT_SEL pin set to %d (Antenna %d selected)", 
                state, state == 0 ? 1 : 2);
    return 0;
}

/**
 * @brief Command handler for setting FEM to low-power TX mode
 */
static int cmd_fem_low_power_tx(const struct shell *shell, size_t argc, char **argv)
{
    int ret = sky66112EnableLowPowerTx();
    if (ret) {
        shell_error(shell, "Failed to set low-power TX mode: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 set to low-power TX mode (Mode 3)");
    return 0;
}

/**
 * @brief Command handler for selecting antenna 1
 */
static int cmd_fem_antenna1(const struct shell *shell, size_t argc, char **argv)
{
    int ret = sky66112SelectAntenna1();
    if (ret) {
        shell_error(shell, "Failed to select antenna 1: %d", ret);
        return ret;
    }

    shell_print(shell, "SKY66112 antenna 1 selected");
    return 0;
}

/**
 * @brief Command handler for displaying FEM mode guide
 */
static int cmd_fem_modes(const struct shell *shell, size_t argc, char **argv)
{
    shell_print(shell, "SKY66112 FEM Operating Modes:");
    shell_print(shell, "----------------------------");
    shell_print(shell, "Mode 0 (Shutdown): CSD=0, all others don't care");
    shell_print(shell, "Mode 1 (Receive): CSD=1, CPS=1, CRX=1, CTX=0, CHL=X");
    shell_print(shell, "Mode 2 (TX High Power): CSD=1, CPS=0, CRX=0, CTX=1, CHL=1");
    shell_print(shell, "Mode 3 (TX Low Power): CSD=1, CPS=0, CRX=0, CTX=1, CHL=0");
    shell_print(shell, "Mode 4 (Bypass): CSD=1, CPS=0, CRX=0, CTX=0, CHL=X");
    shell_print(shell, "");
    shell_print(shell, "Antenna Selection:");
    shell_print(shell, "ANT_SEL=0: Antenna 1 selected");
    shell_print(shell, "ANT_SEL=1: Antenna 2 selected");
    return 0;
}

/**
 * @brief Command handler for setting all FEM pins at once
 */
static int cmd_fem_all_pins(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 7) {
        shell_error(shell, "Usage: %s <csd> <cps> <crx> <ctx> <chl> <ant_sel>", argv[0]);
        shell_print(shell, "All values should be 0 or 1");
        return -EINVAL;
    }

    uint8_t csd = atoi(argv[1]);
    uint8_t cps = atoi(argv[2]);
    uint8_t crx = atoi(argv[3]);
    uint8_t ctx = atoi(argv[4]);
    uint8_t chl = atoi(argv[5]);
    uint8_t ant_sel = atoi(argv[6]);

    /* Check all values are valid */
    if ((csd != 0 && csd != 1) || 
        (cps != 0 && cps != 1) ||
        (crx != 0 && crx != 1) ||
        (ctx != 0 && ctx != 1) ||
        (chl != 0 && chl != 1) ||
        (ant_sel != 0 && ant_sel != 1)) {
        shell_error(shell, "Invalid state value(s). All must be 0 or 1");
        return -EINVAL;
    }

    /* Set all pins */
    sky66112SetCsd(csd);
    sky66112SetCps(cps);
    sky66112SetCrx(crx);
    sky66112SetCtx(ctx);
    sky66112SetChl(chl);
    sky66112SetAntSel(ant_sel);

    shell_print(shell, "SKY66112 pins set: CSD=%d, CPS=%d, CRX=%d, CTX=%d, CHL=%d, ANT_SEL=%d",
                csd, cps, crx, ctx, chl, ant_sel);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(blt_gpio_test_cmds,
    SHELL_CMD(csd, NULL, "Set FEM CSD pin (0|1)", cmd_fem_csd),
    SHELL_CMD(cps, NULL, "Set FEM CPS pin (0|1)", cmd_fem_cps),
    SHELL_CMD(crx, NULL, "Set FEM CRX pin (0|1)", cmd_fem_crx),
    SHELL_CMD(ctx, NULL, "Set FEM CTX pin (0|1)", cmd_fem_ctx),
    SHELL_CMD(chl, NULL, "Set FEM CHL pin (0|1)", cmd_fem_chl),
    SHELL_CMD(ant_sel, NULL, "Set FEM ANT_SEL pin (0|1)", cmd_fem_ant_sel),
    SHELL_CMD(all_pins, NULL, "Set all FEM pins <csd> <cps> <crx> <ctx> <chl> <ant_sel>", cmd_fem_all_pins),
    SHELL_CMD(low_power_tx, NULL, "Set FEM to low-power TX mode", cmd_fem_low_power_tx),
    SHELL_CMD(antenna1, NULL, "Select FEM antenna 1", cmd_fem_antenna1),
    SHELL_CMD(modes, NULL, "Display FEM operating modes guide", cmd_fem_modes),
    SHELL_SUBCMD_SET_END
);

/* Register the command */
SHELL_CMD_REGISTER(blt_gpio, &blt_gpio_test_cmds, "Board Level Test: GPIO functions", NULL);
