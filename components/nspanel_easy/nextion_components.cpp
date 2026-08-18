// nextion_components.cpp

#include "nextion_components.h"
#include <algorithm>
#include <cstring>

namespace esphome::nspanel_easy {

esphome::nextion::Nextion *nextion_display = nullptr;
esphome::text_sensor::TextSensor *detailed_entity_sensor = nullptr;

NSPanelEasyNextionComponent extractNextionComponent(const std::string &input, const std::string &defaultPage) {
  NSPanelEasyNextionComponent result{};
  size_t dotPos = input.find(".");

  if (dotPos != std::string::npos) {
    // Handling special case and standard extraction
    strncpy(result.page, input.substr(0, std::min<size_t>(dotPos, 14)).c_str(), 14);
    result.page[14] = '\0';  // Ensure null termination
    strncpy(result.component_id, input.substr(dotPos + 1, 14).c_str(), 14);
    result.component_id[14] = '\0';  // Ensure null termination
  } else {
    // Default page case
    strncpy(result.page, defaultPage.c_str(), 14);
    result.page[14] = '\0';  // Ensure null termination
    strncpy(result.component_id, input.c_str(), 14);
    result.component_id[14] = '\0';  // Ensure null termination
  }

  return result;
}

}  // namespace esphome::nspanel_easy
