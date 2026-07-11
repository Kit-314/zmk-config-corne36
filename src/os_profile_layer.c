/*
 * os_profile_layer.c — derive the OS base layer from the active BLE profile.
 * SPDX-License-Identifier: MIT
 *
 * Profiles 0/1 = macOS hosts -> LINUX layer off.
 * Profile  2   = Linux host  -> LINUX layer on.
 *
 * The OS base layer is never stored: it is always computed from the active
 * BLE profile, which ZMK itself persists across power-off. That closes the
 * "switch off to charge -> reboots into Mac mode while bonded to the Linux
 * box" gap that a keymap-only (&to in the BtSel macros) solution leaves.
 *
 * Compiled only on the split central (see CMakeLists.txt) — the central owns
 * the BLE profiles and evaluates the keymap. Two triggers:
 *  - zmk_ble_active_profile_changed: BT_SEL presses / any runtime change;
 *  - a delayed one-shot at boot: covers the profile restored from settings.
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>
#include <zmk/keymap.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define OS_LINUX_PROFILE 2
#define OS_LINUX_LAYER_ID 1 /* LINUX layer = second layer in the keymap DTS */

static void apply_os_layer(uint8_t profile_index) {
    if (profile_index == OS_LINUX_PROFILE) {
        LOG_INF("profile %d -> LINUX base on", profile_index);
        zmk_keymap_layer_activate(OS_LINUX_LAYER_ID, false);
    } else {
        LOG_INF("profile %d -> LINUX base off", profile_index);
        zmk_keymap_layer_deactivate(OS_LINUX_LAYER_ID, false);
    }
}

static int os_layer_listener(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev =
        as_zmk_ble_active_profile_changed(eh);
    if (ev != NULL) {
        apply_os_layer(ev->index);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(os_profile_layer, os_layer_listener);
ZMK_SUBSCRIPTION(os_profile_layer, zmk_ble_active_profile_changed);

static void os_layer_boot_work_cb(struct k_work *work) {
    apply_os_layer(zmk_ble_active_profile_index());
}

static K_WORK_DELAYABLE_DEFINE(os_layer_boot_work, os_layer_boot_work_cb);

static int os_layer_boot_init(void) {
    /* Delayed so BLE/settings init has definitely restored the saved profile
     * before we read it. */
    k_work_schedule(&os_layer_boot_work, K_SECONDS(2));
    return 0;
}

SYS_INIT(os_layer_boot_init, APPLICATION, 99);
