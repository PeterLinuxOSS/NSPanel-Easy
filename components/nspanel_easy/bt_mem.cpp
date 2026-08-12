// bt_mem.cpp

#include "bt_mem.h"

#ifdef USE_ESP32_BLE

#include <esp_bt.h>

namespace esphome::nspanel_easy {

bool bt_controller_is_idle() {
#ifndef CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
  return esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE;
#else   // CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
  // ESP-Hosted has no local BT controller — nothing to wait for.
  return true;
#endif  // CONFIG_ESP_HOSTED_ENABLE_BT_BLUEDROID
}

esp_err_t release_bt_memory() { return esp_bt_controller_mem_release(ESP_BT_MODE_BTDM); }

}  // namespace esphome::nspanel_easy

#endif  // USE_ESP32_BLE
