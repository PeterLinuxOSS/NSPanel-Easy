// page_home.cpp

#ifdef NSPANEL_EASY_PAGE_HOME

#include "page_home.h"

#ifdef NSPANEL_EASY_SUBSCRIBE

#include <cctype>
#include <cinttypes>
#include <cstdio>

#include "esphome/components/nextion/nextion.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "nextion_components.h"
#include "text.h"

#endif  // NSPANEL_EASY_SUBSCRIBE

namespace esphome::nspanel_easy {

#ifdef NSPANEL_EASY_SUBSCRIBE

static const char *const TAG = "nspanel.page.home.sub";

HomeButtonState home_button_states[HOME_BUTTON_COUNT] = {};

bool indoor_temp_bound = false;
bool indoor_temp_valid = false;
std::function<void()> indoor_temp_fallback;

/// @brief Last rendered text for each subscribed temperature component.
static char indoor_temp_text[TEMP_TEXT_LEN] = {};
static char outdoor_temp_text[TEMP_TEXT_LEN] = {};

/// @brief Whether the outdoor temperature component is currently shown.
static Visibility outdoor_temp_shown = Visibility::UNKNOWN;

/**
 * @brief Parse the slot index out of a custom button component name.
 *
 * Used by home_sub_render() as a probe: a component that is not a custom button
 * is an ordinary outcome here, so no warning is emitted. The dispatcher warns
 * once, after every target has been tried.
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

/**
 * @brief Render a subscription binding onto a home custom button.
 *
 * Unlike a chip, a bound custom button is always visible: it renders the active
 * or the inactive appearance according to the classified state rather than
 * appearing and disappearing.
 *
 * @param binding The binding being rendered.
 * @param rt Runtime state, including blueprint-supplied appearance.
 * @param state Effective state string; hvac_action for climate when usable.
 * @param idx Zero-based slot index, already validated by the caller.
 */
static void home_button_render(const SubBinding &binding, const SubRuntime &rt, const char *state, uint8_t idx) {
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
  const size_t icon_len = strlen(icon);
  if (icon_len >= sizeof(button.icon)) {
    // A truncated copy would leave the strcmp below permanently unequal, so the
    // renderer would write to the Nextion on every state change.
    ESP_LOGW(TAG, "%s icon does not fit (%zu bytes); skipping", binding.component, icon_len);
    return;
  }

  const bool icon_changed = (strcmp(button.icon, icon) != 0);
  if (icon_changed) {
    memcpy(button.icon, icon, icon_len + 1);  // Length checked above, so the null terminator fits
  }

  const bool color_changed = (button.color != color);
  button.color = color;

  ESP_LOGV(TAG, "%s: icon='%s' color=%" PRIu16 " home=%s appearance=%s", binding.component, icon, color,
           YESNO(is_home_page()), YESNO(rt.has_appearance));

  if (!is_home_page()) {
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

#ifdef NSPANEL_EASY_USE_WEATHER

void home_weather_resolve() {
  if (nextion_display == nullptr) {
    return;  // Boot has not handed the display over yet
  }

  // is_new_device is always false: the "Easy" TFT project is abandoned.
  const WeatherPicVariant &variant =
      select_weather_variant(get_weather_pics(weather_condition_index), false, current_theme != ThemeMode::LIGHT);
  const uint16_t pic = sun_info.is_up ? variant.sun_up : variant.sun_down;

  // Index 0 resolves to a blank picture in both themes, so an unknown or
  // not-yet-received condition needs no visibility handling of its own.
  ESP_LOGD(TAG, "Update weather pic: %" PRIu16 " (condition %" PRIu8 ", sun up: %s)", pic, weather_condition_index,
           YESNO(sun_info.is_up));
  nextion_display->set_component_pic(hmi::home::WEATHER.name, static_cast<uint8_t>(pic));
}

/**
 * @brief Render a subscription binding onto the home weather picture.
 *
 * The picture depends on the condition, the sun elevation and the active theme,
 * so the condition is stored and the shared resolver does the rest. The same
 * resolver runs on theme changes and on sunrise/sunset.
 *
 * @param state Raw weather condition string from Home Assistant.
 */
static void home_weather_render(const char *state) {
  const uint8_t index = get_weather_index(state);
  if (index == weather_condition_index) {
    return;  // Nothing changed; avoid a redundant Nextion write
  }
  weather_condition_index = index;
  home_weather_resolve();
}

#endif  // NSPANEL_EASY_USE_WEATHER

/**
 * @brief Format a temperature the way every other page in the project does.
 *
 * Fahrenheit renders without decimals, Celsius with one, matching
 * display_embedded_temp() and the climate page. The value is rendered exactly
 * as Home Assistant reported it: no unit conversion is attempted, since a
 * subscribed entity carries no reliable native unit.
 *
 * @param value Temperature as reported by Home Assistant.
 * @param out Destination buffer.
 * @param size Size of the destination buffer.
 */
static void format_temperature(float value, char *out, size_t size) {
  char buffer[TEMP_TEXT_LEN];
  const char *separator = (units_separator_str != nullptr) ? units_separator_str->c_str() : "";
#if NSPANEL_EASY_HW_TEMPERATURE_IS_FAHRENHEIT
  snprintf(buffer, sizeof(buffer), "%.0f%s\u00b0F", value, separator);
#else   // Celsius
  snprintf(buffer, sizeof(buffer), "%.1f%s\u00b0C", value, separator);
#endif  // NSPANEL_EASY_HW_TEMPERATURE_IS_FAHRENHEIT

  // Guarded indexing: an empty separator would be undefined behaviour here.
  const char decimal =
      (decimal_separator_str == nullptr || decimal_separator_str->empty()) ? '.' : (*decimal_separator_str)[0];
  const std::string adjusted = adjustDecimalSeparator(buffer, decimal);
  strncpy(out, adjusted.c_str(), size - 1);
  out[size - 1] = '\0';
}

/**
 * @brief Render a subscription binding onto the indoor temperature.
 *
 * The subscription owns the component only while it has a usable number. When
 * the bound entity has no value, display_embedded_temp() takes back over and
 * shows the panel's own sensor, so the component is never left blank and needs
 * no visibility handling of its own.
 *
 * @param state Raw state string, or the current_temperature attribute for climate.
 */
static void home_indoor_temp_render(const char *state) {
  indoor_temp_bound = true;

  const auto value = parse_number<float>(state);
  if (!value.has_value()) {
    // Unavailable, unknown, empty or simply not a number. Hand the component
    // back rather than waiting for the next embedded sensor publication, which
    // can be minutes away.
    if (indoor_temp_valid) {
      indoor_temp_valid = false;
      indoor_temp_text[0] = '\0';
      ESP_LOGD(TAG, "Indoor temperature unavailable; falling back to the embedded sensor");
      if (indoor_temp_fallback) {
        indoor_temp_fallback();
      }
    }
    return;
  }

  indoor_temp_valid = true;
  char text[TEMP_TEXT_LEN];
  format_temperature(*value, text, sizeof(text));
  if (strcmp(indoor_temp_text, text) == 0) {
    return;  // Nothing changed; avoid a redundant Nextion write
  }
  strncpy(indoor_temp_text, text, sizeof(indoor_temp_text) - 1);
  indoor_temp_text[sizeof(indoor_temp_text) - 1] = '\0';

  if (!is_home_page()) {
    return;  // Shadow is up to date; the repaint on entry will draw it
  }
  nextion_display->set_component_text(hmi::home::INDR_TEMP.name, indoor_temp_text);
}

/**
 * @brief Render a subscription binding onto the outdoor temperature.
 *
 * Shown while the bound entity reports a usable number, hidden otherwise. There
 * is no local fallback for this component, so hiding it is the only sensible
 * response to an unavailable entity.
 *
 * @param state Raw state string, or the temperature attribute for weather.
 */
static void home_outdoor_temp_render(const char *state) {
  const auto value = parse_number<float>(state);
  const bool visible = value.has_value();

  if (visible) {
    char text[TEMP_TEXT_LEN];
    format_temperature(*value, text, sizeof(text));
    if (strcmp(outdoor_temp_text, text) != 0) {
      strncpy(outdoor_temp_text, text, sizeof(outdoor_temp_text) - 1);
      outdoor_temp_text[sizeof(outdoor_temp_text) - 1] = '\0';
      if (is_home_page()) {
        nextion_display->set_component_text(hmi::home::OUTDOOR_TEMP.name, outdoor_temp_text);
      }
    }
  } else {
    outdoor_temp_text[0] = '\0';
  }

  const Visibility wanted = visible ? Visibility::SHOWN : Visibility::HIDDEN;
  if (outdoor_temp_shown == wanted) {
    return;  // Visibility unchanged
  }
  outdoor_temp_shown = wanted;

  // The TFT restores visibility from vis_<component> on page entry, so the
  // variable is always written; the vis command only reaches a visible page.
  nextion_display->send_command_printf("vis_outdoor_temp=%" PRIu8, static_cast<uint8_t>(visible));
  if (is_home_page()) {
    nextion_display->set_component_visibility("outdoor_temp", visible);
  }
}

void home_sub_render(const SubBinding &binding, const SubRuntime &rt, const char *state, bool visible) {
  if (nextion_display == nullptr) {
    return;  // Boot has not handed the display over yet
  }

  const uint8_t idx = parse_home_button_index(binding.component);
  if (idx != UINT8_MAX) {
    home_button_render(binding, rt, state, idx);
    return;
  }
#ifdef NSPANEL_EASY_USE_WEATHER
  if (strcmp(binding.component, "weather") == 0) {
    home_weather_render(state);
    return;
  }
#endif  // NSPANEL_EASY_USE_WEATHER
  if (strcmp(binding.component, "indr_temp") == 0) {
    home_indoor_temp_render(state);
    return;
  }
  if (strcmp(binding.component, "outdoor_temp") == 0) {
    home_outdoor_temp_render(state);
    return;
  }
  ESP_LOGW(TAG, "'%s' is not a subscribable home component", binding.component);
}

void home_button_repaint() {
  if (nextion_display == nullptr) {
    ESP_LOGE(TAG, "Missing Nextion display pointer");
    return;
  }
  if (!is_home_page()) {
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

  // Temperatures are repainted from the same shadow, for the same reason: the
  // Nextion does not retain what was drawn on a page that has been left.
  if (indoor_temp_bound && indoor_temp_valid && indoor_temp_text[0] != '\0') {
    nextion_display->set_component_text(hmi::home::INDR_TEMP.name, indoor_temp_text);
  }
  if (outdoor_temp_shown != Visibility::UNKNOWN) {
    // Re-asserted rather than assumed: a TFT reset without an ESPHome restart
    // returns vis_outdoor_temp to its default while this shadow still holds the
    // last value, and the renderer would then suppress every further write.
    const bool visible = (outdoor_temp_shown == Visibility::SHOWN);
    nextion_display->send_command_printf("vis_outdoor_temp=%" PRIu8, static_cast<uint8_t>(visible));
    nextion_display->set_component_visibility("outdoor_temp", visible);
    if (visible && outdoor_temp_text[0] != '\0') {
      nextion_display->set_component_text(hmi::home::OUTDOOR_TEMP.name, outdoor_temp_text);
    }
  }
}

#endif  // NSPANEL_EASY_SUBSCRIBE

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_PAGE_HOME
