SHELL_STATIC_SUBCMD_SET_CREATE(blt_gpio_test_cmds,
  TODO
	SHELL_SUBCMD_SET_END
);

// Register the command
SHELL_CMD_REGISTER(blt_gpio, &blt_gpio_test_cmds, "Board Level Test: GPIO functions", NULL);
