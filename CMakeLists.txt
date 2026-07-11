# Custom firmware sources for this config repo (picked up because
# zephyr/module.yml declares this repo a Zephyr module with cmake: .).
# The OS-profile-layer listener only makes sense on the split central —
# the half that owns BLE profiles and evaluates the keymap.
if(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources(app PRIVATE src/os_profile_layer.c)
endif()
