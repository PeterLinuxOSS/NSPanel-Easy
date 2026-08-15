// chips.h

#pragma once

#ifdef NSPANEL_EASY_CHIPS

#include <cstdint>
#include <cstring>
#include <string>

#ifdef NSPANEL_EASY_SUBSCRIBE
#include "api_subscriptions.h"
#endif  // NSPANEL_EASY_SUBSCRIBE

/**
 * @file chips.h
 * @brief Chip state shadow for the NSPanel status chip bar.
 *
 * Defines the ChipState struct and the chip_states array that ESPHome uses
 * to remember each chip's last-known icon, color, and visibility so that
 * home and screensaver pages can be re-rendered at any time without waiting
 * for the blueprint to resend data.
 *
 * Chip index mapping (matches CHIP_NAMES order):
 *   0 — chip_relay1
 *   1 — chip_relay2
 *   2 — chip_climate
 *   3 — chip01
 *   4 — chip02
 *   5 — chip03
 *   6 — chip04
 *   7 — chip05
 *   8 — chip06
 *   9 — chip07
 *
 * Slots 0–2 are driven locally by the relay and embedded-thermostat logic and
 * keep their own persisted appearance, so they render without Home Assistant.
 * Slots 3–9 may be bound to a Home Assistant subscription instead, in which
 * case blueprint pushes for them are ignored.
 *
 * Component names are unscoped (no page prefix) so that a single write lands
 * on whichever page (home or screensaver) is currently visible.
 */

namespace esphome::nspanel_easy {

/// @brief Total number of chip slots managed by this module.
static constexpr uint8_t CHIP_COUNT = 10;

/**
 * @brief Shadow state for a single chip.
 *
 * Stores the last icon, color, and visibility pushed by the blueprint or
 * by internal relay/climate logic. Used to re-render chips on page load
 * without requiring a new HA push.
 */
struct ChipState {
  char icon[4];     ///< UTF-8 MDI codepoint (1–3 bytes + null terminator)
  uint16_t color;   ///< RGB565 foreground color
  bool visible;     ///< Whether the chip should be shown
  bool subscribed;  ///< Driven by a direct HA state subscription; ignore blueprint pushes
};

/**
 * @brief Unscoped Nextion component names for each chip slot.
 *
 * Index order matches chip_states[]. These names are intentionally unscoped
 * so that Nextion resolves them against the currently visible page, which
 * must be either home or screensaver before any render call is issued.
 */
static constexpr const char *CHIP_NAMES[CHIP_COUNT] = {
    "chip_relay1",   ///< Index 0
    "chip_relay2",   ///< Index 1
    "chip_climate",  ///< Index 2
    "chip01",        ///< Index 3
    "chip02",        ///< Index 4
    "chip03",        ///< Index 5
    "chip04",        ///< Index 6
    "chip05",        ///< Index 7
    "chip06",        ///< Index 8
    "chip07",        ///< Index 9
};

// Named indices for direct access by relay and climate logic.
static constexpr uint8_t CHIP_IDX_RELAY1 = 0;   ///< chip_relay1 index in chip_states
static constexpr uint8_t CHIP_IDX_RELAY2 = 1;   ///< chip_relay2 index in chip_states
static constexpr uint8_t CHIP_IDX_CLIMATE = 2;  ///< chip_climate index in chip_states
static constexpr uint8_t CHIP_IDX_USER = 3;     ///< First user chip (chip01) index in chip_states

/// @brief Shadow state array; one entry per chip slot.
extern ChipState chip_states[CHIP_COUNT];

extern bool is_chips_page;

/**
 * @brief Find the chip index for a given component name.
 *
 * @param name Unscoped Nextion component name (e.g. "chip01").
 * @return Index into chip_states[] / CHIP_NAMES[], or UINT8_MAX if not found.
 */
inline uint8_t find_chip_index(const char *name) {
  for (uint8_t i = 0; i < CHIP_COUNT; ++i) {
    if (strcmp(name, CHIP_NAMES[i]) == 0) {
      return i;
    }
  }
  return UINT8_MAX;  // sentinel: not found
}

/**
 * @brief Find the chip index for a given component name.
 *
 * @param name Unscoped Nextion component name (e.g. "chip01").
 * @return Index into chip_states[] / CHIP_NAMES[], or UINT8_MAX if not found.
 */
inline uint8_t find_chip_index(const std::string &name) { return find_chip_index(name.c_str()); }

#ifdef NSPANEL_EASY_SUBSCRIBE

/**
 * @brief Render a subscription binding onto a chip.
 *
 * Registered for the "chips" page in sub_resolve_renderer(). Receives an
 * already-classified state and decides only how to draw it.
 *
 * @param binding The binding being rendered.
 * @param rt Runtime state, including blueprint-supplied appearance.
 * @param state Effective state string; hvac_action for climate when usable.
 * @param visible Whether the chip should be shown.
 */
void chip_sub_render(const SubBinding &binding, const SubRuntime &rt, const char *state, bool visible);

#endif  // NSPANEL_EASY_SUBSCRIBE

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_CHIPS
