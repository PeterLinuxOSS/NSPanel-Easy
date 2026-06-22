// page_qrcode.h

#pragma once

#ifdef NSPANEL_EASY_PAGE_QRCODE

#include "nextion_components.h"  // For HMIComponent
#include "pages.h"               // For page_names and get_page_id

/**
 * @file page_qrcode.h
 * @brief Nextion component definitions for the QR Code page.
 *
 * This file contains all component constants specific to the QR Code page
 * of the NSPanel interface.
 */

namespace esphome::nspanel_easy {
namespace hmi::qrcode {

/**
 * @namespace qrcode
 * @brief Components for the QR Code page.
 *
 * Component ID mapping for the QR Code page
 * Based on the Nextion HMI design file.
 * Note: All components are local scope, so names don't include page prefix.
 */

// Button components
constexpr HMIComponent BUTTON_BACK = {"button_back", 1};  ///< Back button (3 chars max) - returns to home

// The QRcode box
constexpr HMIComponent QR = {"qr", 2};  ///< The QRcode itself

// Display components
constexpr HMIComponent ICON_STATE = {"icon_state", 3};  ///< QR code icon (4 chars max)

// Display components
constexpr HMIComponent PAGE_LABEL = {"page_label", 4};  ///< QR code label text (30 chars max)

// All visual components for iteration
constexpr HMIComponent ALL[] = {PAGE, BUTTON_BACK, QR, ICON_STATE, PAGE_LABEL};

constexpr size_t COMPONENT_COUNT = sizeof(ALL) / sizeof(ALL[0]);

}  // namespace hmi::qrcode
}  // namespace esphome::nspanel_easy

#endif  // NSPANEL_EASY_PAGE_QRCODE
