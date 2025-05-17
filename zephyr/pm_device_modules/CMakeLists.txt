target_sources_ifdef(CONFIG_SHARED_PM_DEVICE_MODULES app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/pm_device_module.c
)
