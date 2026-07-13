# Custom firmware sources for this config repo (picked up because
# zephyr/module.yml declares this repo a Zephyr module with cmake: .).
# The OS-profile-layer listener only makes sense on the split central —
# the half that owns BLE profiles and evaluates the keymap. The ZMK_BLE
# guard also matters: settings_reset builds keep the central flag but strip
# BLE support, so without it the listener's zmk_ble_* symbols fail to link.
if(CONFIG_ZMK_SPLIT_ROLE_CENTRAL AND CONFIG_ZMK_BLE)
  target_sources(app PRIVATE src/os_profile_layer.c)
endif()
