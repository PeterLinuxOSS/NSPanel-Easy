// page_home.cpp

#ifdef NSPANEL_EASY_PAGE_HOME

#include "page_home.h"

#ifdef NSPANEL_EASY_SUBSCRIBE
#include <cctype>
#include <cinttypes>
#include <cstdio>

#include "esphome/components/nextion/nextion.h"
#include "esphome/core/log.h"

#include "nextion_components.h"
#endif  // NSPANEL_EASY_SUBSCRIBE

namespace esphome::nspanel_easy {

bool is_home_page = false;

#ifdef NSPANEL_EASY_SUBSCRIBE

static const char *const TAG = "nspanel.page.home.sub";

HomeButtonState home_button_states[HOME_BUTTON_COUNT] = {};

/**
 * @brief Parse the slot index out of a custom button component name.
 *
 * @param component Component name, expected as "button%02u".
 * @return Zero-based index into home_button_states[], or UINT8_MAX if invalid.
 */
static uint8_t parse_home_button_index(const char *component) {
  constexpr uint8_t PREFIX_LEN = 6;  // strlen("button")
  if (strlen(component) != PREFIX_LEN + 2 || strncmp(component, "button", PREFIX_LEN) != 0) {
    return UINT8_MAX;
  }
  const char tens = component[PREFIX_LEN];
  const char units = component[PREFIX_LEN + 1];
  if (!std::isdigit(static_cast<unsigned char>(tens)) || !std::isdigit(static_cast<unsigned char>(units))) {
    return UINT8_MAX;
  }
  const uint8_t number = static_cast<uint8_t>(((tens - '0') * 10) + (units - '0'));
  if (number < 1 || number > HOME_BUTTON_COUNT) {
    return UINT8_MAX;
  }
  return static_cast<uint8_t>(number - 1);
}

void home_button_sub_render(const SubBinding &binding, const SubRuntime &rt, const char *state, bool visible) {
  const uint8_t idx = parse_home_button_index(binding.component);
  if (idx == UINT8_MAX) {
    ESP_LOGW(TAG, "'%s' is not a home custom button", binding.component);
    return;
  }
  if (nextion_display == nullptr) {
    return;  // Boot has not handed the display over yet
  }

  HomeButtonState &button = home_button_states[idx];
  button.bound = true;  // Blocks blueprint pushes for this slot

  // Nothing to draw until a binding push has supplied appearance. Appearance is
  // not persisted, so a button stays as the blueprint last left it until then.
  if (!rt.has_appearance) {
    return;
  }

  // Resolve unconditionally, even while the home page is not showing: the page
  // is repainted from this shadow on entry, without asking Home Assistant.
  // SUB_STATE_TRANSITIONAL and SUB_STATE_NEITHER both fall to the inactive
  // appearance, matching how an unavailable entity renders today.
  const bool active = (rt.last_state == SUB_STATE_ON);
  const char *icon = active ? rt.icon_on : rt.icon_off;
  uint16_t color = active ? rt.color_on : rt.color_off;

  // An icon pushed by the blueprint wins; otherwise resolve from the domain and
  // state, which is what the multi-state domains rely on.
  if (icon[0] == '\0') {
    const SubAppearance look = resolve_sub_appearance(static_cast<SubDomain>(binding.domain), binding.device_class,
                                                      state, rt.color_on, rt.color_off);
    if (look.icon != nullptr) {
      icon = look.icon;
      color = look.color;
    }
  }
  if (icon[0] == '\0') {
    ESP_LOGW(TAG, "%s has no icon", binding.component);
    return;
  }
  if (strlen(icon) >= sizeof(button.icon)) {
    ESP_LOGW(TAG, "%s icon does not fit (%zu bytes); skipping", binding.component, strlen(icon));
    return;
  }

  const bool icon_changed = (strcmp(button.icon, icon) != 0);
  if (icon_changed) {
    strncpy(button.icon, icon, sizeof(button.icon) - 1);
    button.icon[sizeof(button.icon) - 1] = '\0';
  }

  const bool color_changed = (button.color != color);
  button.color = color;

  ESP_LOGV(TAG, "%s: icon='%s' color=%" PRIu16 " home=%s appearance=%s", binding.component, icon, color,
           YESNO(is_home_page), YESNO(rt.has_appearance));

  if (!is_home_page) {
    return;  // Shadow is up to date; the repaint on entry will draw it
  }
  if (icon_changed) {
    nextion_display->set_component_text(binding.component, button.icon);
  }
  if (color_changed) {
    nextion_display->set_component_font_color(binding.component, color);
  }
  if (!button.shown) {
    // A bound button is always visible. The blueprint used to send this with
    // every icon push; nothing else sets it now.
    button.shown = true;
    nextion_display->set_component_visibility(binding.component, true);
  }
}

void home_button_repaint() {
  if (nextion_display == nullptr) {
    ESP_LOGE(TAG, "Missing Nextion display pointer");
    return;
  }
  if (!is_home_page) {
    return;  // Unscoped names resolve against the visible page only
  }
  char component[9] = {};  // "buttonNN" + null terminator
  for (uint8_t idx = 0; idx < HOME_BUTTON_COUNT; ++idx) {
    HomeButtonState &button = home_button_states[idx];
    if (!button.bound || button.icon[0] == '\0') {
      continue;  // Unbound slots stay with whatever the blueprint drew
    }
    snprintf(component, sizeof(component), "button%02" PRIu8, static_cast<uint8_t>(idx + 1));
    nextion_display->set_component_text(component, button.icon);
    nextion_display->set_component_font_color(component, button.color);
    nextion_display->set_component_visibility(component, true);
    button.shown = true;
  }  // for each custom button slot
}

#endif  // NSPANEL_EASY_SUBSCRIBE

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_PAGE_HOME