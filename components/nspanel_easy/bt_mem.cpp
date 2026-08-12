// bt_mem.cpp

#include "bt_mem.h"

#ifdef USE_ESP32_BLE

#include <esp_bt.h>

namespace esphome::nspanel_easy {

esp_err_t release_bt_memory() { return esp_bt_controller_mem_release(ESP_BT_MODE_BTDM); }

}  // namespace esphome::nspanel_easy

#endif  // USE_ESP32_BLE
