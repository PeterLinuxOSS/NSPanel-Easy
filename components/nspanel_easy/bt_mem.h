// bt_mem.h - Bluetooth controller memory release helper

#pragma once

#include "esphome/core/defines.h"

#ifdef USE_ESP32_BLE

#include <esp_err.h>

namespace esphome::nspanel_easy {

/**
 * @brief Checks whether the Bluetooth controller is fully deinitialized.
 *
 * ESP32BLE::disable() only requests the shutdown; the controller is torn down
 * later, from the main loop. is_active() turns false immediately and is
 * therefore not a valid gate for releasing the controller memory.
 *
 * @return true when the controller reached the IDLE state.
 */
bool bt_controller_is_idle();

/**
 * @brief Releases the Bluetooth controller memory (BT Classic + BLE) back to the heap.
 *
 * ESP32BLE::disable() deinitializes the controller but does not return its
 * static allocations to the heap. This must be called explicitly when the RAM
 * is needed elsewhere (e.g. TFT upload). The release is irreversible until reboot.
 *
 * @return ESP_OK on success, otherwise an esp_err_t error code.
 */
esp_err_t release_bt_memory();

}  // namespace esphome::nspanel_easy

#endif  // USE_ESP32_BLE
