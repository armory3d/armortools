#pragma once

#include <kinc/global.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>
#include "HIDGamepad.h"

// Maximum number of devices supported
// Corresponds to size of Kore::Gamepad array
static const int KINC_MAX_HID_DEVICES = 12;

// Slots to hold details on connected devices
struct HIDManagerDeviceRecord {
	bool connected;        // = false;
	IOHIDDeviceRef device; // = NULL;
	struct HIDGamepad pad;
};

struct HIDManager {
	IOHIDManagerRef managerRef;
	struct HIDManagerDeviceRecord devices[KINC_MAX_HID_DEVICES];
};

void HIDManager_init(struct HIDManager *manager);
void HIDManager_destroy(struct HIDManager *manager);
