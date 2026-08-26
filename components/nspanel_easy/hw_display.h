// hw_display.h

#pragma once

#ifdef NSPANEL_EASY_HW_DISPLAY

#include <cstdint>
#include <vector>
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "pages.h"
#include "base.h"

namespace esphome::nspanel_easy {

/**
 * @brief Display theme modes.
 *
 * Stored as a single byte (uint8_t) for use in Nextion EEPROM
 * and ESPHome globals with restore_value.
 */
enum class ThemeMode : uint8_t {
  DARK = 0,   ///< Dark theme  - light text on dark background
  LIGHT = 1,  ///< Light theme - dark text on light background
};
extern ThemeMode current_theme;  ///< Active display theme

extern uint8_t brightness_current;
extern bool display_entity_keep;
extern uint8_t display_mode_eeprom;
extern bool display_portrait;
extern bool display_valid;

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_HW_DISPLAY
