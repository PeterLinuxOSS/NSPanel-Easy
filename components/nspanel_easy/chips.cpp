// chips.cpp

#ifdef NSPANEL_EASY_CHIPS

#include "chips.h"
#include <string>

#ifdef NSPANEL_EASY_SUBSCRIBE
#include "nextion_components.h"
#include "esphome/components/nextion/nextion.h"
#include "esphome/core/log.h"
#endif  // NSPANEL_EASY_SUBSCRIBE

namespace esphome::nspanel_easy {

ChipState chip_states[CHIP_COUNT] = {};
bool is_chips_page = false;

#ifdef NSPANEL_EASY_SUBSCRIBE

static const char *const TAG = "nspanel.chips.sub";

void chip_sub_render(const SubBinding &binding, const SubRuntime &rt, const char *state, bool visible) {
  const uint8_t idx = find_chip_index(binding.component);
  if (idx == UINT8_MAX) {
    ESP_LOGW(TAG, "'%s' is not a chip component", binding.component);
    return;
  }

  if (nextion_display == nullptr) {
    return;  // Boot has not handed the display over yet
  }

  ChipState &the_chip = chip_states[idx];
  the_chip.subscribed = true;  // Blocks blueprint pushes for this slot

  // Nothing to draw until the blueprint has pushed appearance for this session.
  // Appearance is not persisted, so a chip stays hidden after a boot until then.
  if (!rt.has_appearance) {
    return;
  }

  if (visible) {
    const bool active = (rt.last_state == SUB_STATE_ON);
    const char *icon = active ? rt.icon_on : rt.icon_off;
    uint16_t color = active ? rt.color_on : rt.color_off;

    // An icon pushed by the blueprint wins; otherwise resolve from the domain
    // and state, which is what the multi-state domains rely on.
    if (icon[0] == '\0') {
      const SubAppearance look = resolve_sub_appearance(static_cast<SubDomain>(binding.domain), binding.device_class,
                                                        state, rt.color_on, rt.color_off);
      if (look.icon != nullptr) {
        icon = look.icon;
        color = look.color;
      }
    }

    if (icon[0] == '\0') {
      ESP_LOGW(TAG, "%s has no icon, keeping it hidden", binding.component);
      return;
    }

    if (strcmp(the_chip.icon, icon) != 0) {
      strncpy(the_chip.icon, icon, sizeof(the_chip.icon) - 1);
      the_chip.icon[sizeof(the_chip.icon) - 1] = '\0';
      if (is_chips_page) {
        nextion_display->set_component_text(CHIP_NAMES[idx], the_chip.icon);
      }
    }
    if (the_chip.color != color) {
      the_chip.color = color;
      if (is_chips_page) {
        nextion_display->set_component_font_color(CHIP_NAMES[idx], color);
      }
    }
  }  // if visible — appearance is left untouched while hidden

  if (the_chip.visible == visible) {
    return;
  }
  the_chip.visible = visible;
  if (is_chips_page) {
    nextion_display->set_component_visibility(CHIP_NAMES[idx], visible);
  }
}

#endif  // NSPANEL_EASY_SUBSCRIBE

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_CHIPS
