#pragma once

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

struct HIDGamepad {
	int padIndex;
	IOHIDDeviceRef hidDeviceRef;
	IOHIDQueueRef hidQueueRef;
	int hidDeviceVendorID;
	int hidDeviceProductID;
	char hidDeviceVendor[64];
	char hidDeviceProduct[64];

	IOHIDElementCookie axis[6];
	IOHIDElementCookie buttons[15];
};

void HIDGamepad_init(struct HIDGamepad *gamepad);
void HIDGamepad_destroy(struct HIDGamepad *gamepad);
void HIDGamepad_bind(struct HIDGamepad *gamepad, IOHIDDeviceRef deviceRef, int padIndex);
void HIDGamepad_unbind(struct HIDGamepad *gamepad);
