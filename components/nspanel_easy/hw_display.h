// hw_display.h

#pragma once

#ifdef NSPANEL_EASY_HW_DISPLAY

#include <cstdint>
#include <functional>
#include <string>
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

/**
 * @brief Optional local handler for a click, tried before it reaches Home Assistant.
 *
 * Assigned from a page package that needs to act on a click itself rather than
 * forward it, currently the home page routing a custom button through the
 * confirmation dialog. Returning true consumes the event; whatever handled it
 * is responsible for re-emitting the click if the user goes ahead.
 *
 * @param page Page that produced the event, as reported by the display.
 * @param event "short_click" or "long_click".
 * @param component Unscoped component name, e.g. "button01".
 * @return true when the click was handled locally and must not reach Home Assistant.
 */
using ClickInterceptor =
    std::function<bool(const std::string &page, const std::string &event, const std::string &component)>;
extern ClickInterceptor click_interceptor;

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_HW_DISPLAY
