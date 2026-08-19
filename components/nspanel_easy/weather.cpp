// weather.cpp

#ifdef NSPANEL_EASY_USE_WEATHER

#include "weather.h"

namespace esphome::nspanel_easy {

SunInfo sun_info = {
    .is_up = true,          // Safe daytime default before the first time sync
    .coords_valid = false,  // Coordinates not yet applied - time proxy active
};

uint8_t weather_condition_index = 0;  // Defaults to fallback until first blueprint update

}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_USE_WEATHER
