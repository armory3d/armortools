#include "HIDGamepad.h"
#include "HIDManager.h"
#include <kinc/error.h>
#include <kinc/input/gamepad.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);
static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender);

static void reset(struct HIDGamepad *gamepad);

static void initDeviceElements(struct HIDGamepad *gamepad, CFArrayRef elements);

static void buttonChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex);
static void axisChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex);

static bool debugButtonInput = false;

static void logButton(int buttonIndex, bool pressed) {
	switch (buttonIndex) {
	case 0:
		kinc_log(KINC_LOG_LEVEL_INFO, "A Pressed %i", pressed);
		break;

	case 1:
		kinc_log(KINC_LOG_LEVEL_INFO, "B Pressed %i", pressed);
		break;

	case 2:
		kinc_log(KINC_LOG_LEVEL_INFO, "X Pressed %i", pressed);
		break;

	case 3:
		kinc_log(KINC_LOG_LEVEL_INFO, "Y Pressed %i", pressed);
		break;

	case 4:
		kinc_log(KINC_LOG_LEVEL_INFO, "Lb Pressed %i", pressed);
		break;

	case 5:
		kinc_log(KINC_LOG_LEVEL_INFO, "Rb Pressed %i", pressed);
		break;

	case 6:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left Stick Pressed %i", pressed);
		break;

	case 7:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right Stick Pressed %i", pressed);
		break;

	case 8:
		kinc_log(KINC_LOG_LEVEL_INFO, "Start Pressed %i", pressed);
		break;

	case 9:
		kinc_log(KINC_LOG_LEVEL_INFO, "Back Pressed %i", pressed);
		break;

	case 10:
		kinc_log(KINC_LOG_LEVEL_INFO, "Home Pressed %i", pressed);
		break;

	case 11:
		kinc_log(KINC_LOG_LEVEL_INFO, "Up Pressed %i", pressed);
		break;

	case 12:
		kinc_log(KINC_LOG_LEVEL_INFO, "Down Pressed %i", pressed);
		break;

	case 13:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left Pressed %i", pressed);
		break;

	case 14:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right Pressed %i", pressed);
		break;

	default:
		break;
	}
}

static bool debugAxisInput = false;

static void logAxis(int axisIndex) {
	switch (axisIndex) {
	case 0:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left stick X");
		break;

	case 1:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left stick Y");
		break;

	case 2:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right stick X");
		break;

	case 3:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right stick Y");
		break;

	case 4:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left trigger");
		break;

	case 5:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right trigger");
		break;

	default:
		break;
	}
}

// Helper function to copy a CFStringRef to a cstring buffer.
// CFStringRef is converted to UTF8 and as many characters as possible are
// placed into the buffer followed by a null terminator.
// The buffer is set to an empty string if the conversion fails.
static void cstringFromCFStringRef(CFStringRef string, char *cstr, size_t clen) {
	cstr[0] = '\0';
	if (string != NULL) {
		char temp[256];
		if (CFStringGetCString(string, temp, 256, kCFStringEncodingUTF8)) {
			temp[kinc_mini(255, (int)(clen - 1))] = '\0';
			strncpy(cstr, temp, clen);
		}
	}
}

void HIDGamepad_init(struct HIDGamepad *gamepad) {
	reset(gamepad);
}

void HIDGamepad_destroy(struct HIDGamepad *gamepad) {
	HIDGamepad_unbind(gamepad);
}

void HIDGamepad_bind(struct HIDGamepad *gamepad, IOHIDDeviceRef inDeviceRef, int inPadIndex) {
	kinc_affirm(inDeviceRef != NULL);
	kinc_affirm(inPadIndex >= 0);
	kinc_affirm(gamepad->hidDeviceRef == NULL);
	kinc_affirm(gamepad->hidQueueRef == NULL);
	kinc_affirm(gamepad->padIndex == -1);

	// Set device and device index
	gamepad->hidDeviceRef = inDeviceRef;
	gamepad->padIndex = inPadIndex;

	// Initialise HID Device
	// ...open device
	IOHIDDeviceOpen(gamepad->hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);

	// ..register callbacks
	IOHIDDeviceRegisterInputValueCallback(gamepad->hidDeviceRef, inputValueCallback, gamepad);
	IOHIDDeviceScheduleWithRunLoop(gamepad->hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	// ...create a queue to access element values
	gamepad->hidQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, gamepad->hidDeviceRef, 32, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(gamepad->hidQueueRef) == IOHIDQueueGetTypeID()) {
		IOHIDQueueStart(gamepad->hidQueueRef);
		IOHIDQueueRegisterValueAvailableCallback(gamepad->hidQueueRef, valueAvailableCallback, gamepad);
		IOHIDQueueScheduleWithRunLoop(gamepad->hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	// ...get all elements (buttons, axes)
	CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(gamepad->hidDeviceRef, NULL, kIOHIDOptionsTypeNone);
	initDeviceElements(gamepad, elementCFArrayRef);

	// ...get device manufacturer and product details
	{
		CFNumberRef vendorIdRef = (CFNumberRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDVendorIDKey));
		CFNumberGetValue(vendorIdRef, kCFNumberIntType, &gamepad->hidDeviceVendorID);

		CFNumberRef productIdRef = (CFNumberRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDProductIDKey));
		CFNumberGetValue(productIdRef, kCFNumberIntType, &gamepad->hidDeviceProductID);

		CFStringRef vendorRef = (CFStringRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDManufacturerKey));
		cstringFromCFStringRef(vendorRef, gamepad->hidDeviceVendor, sizeof(gamepad->hidDeviceVendor));

		CFStringRef productRef = (CFStringRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDProductKey));
		cstringFromCFStringRef(productRef, gamepad->hidDeviceProduct, sizeof(gamepad->hidDeviceProduct));
	}

	kinc_log(KINC_LOG_LEVEL_INFO, "HIDGamepad.bind: <%p> idx:%d [0x%x:0x%x] [%s] [%s]", inDeviceRef, gamepad->padIndex, gamepad->hidDeviceVendorID,
	         gamepad->hidDeviceProductID, gamepad->hidDeviceVendor, gamepad->hidDeviceProduct);
}

static void initDeviceElements(struct HIDGamepad *gamepad, CFArrayRef elements) {
	kinc_affirm(elements != NULL);

	for (CFIndex i = 0, count = CFArrayGetCount(elements); i < count; ++i) {
		IOHIDElementRef elementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		IOHIDElementType elemType = IOHIDElementGetType(elementRef);

		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);

		uint32_t usagePage = IOHIDElementGetUsagePage(elementRef);
		uint32_t usage = IOHIDElementGetUsage(elementRef);

		// Match up items
		switch (usagePage) {
		case kHIDPage_GenericDesktop:
			switch (usage) {
			case kHIDUsage_GD_X: // Left stick X
				gamepad->axis[0] = cookie;
				break;
			case kHIDUsage_GD_Y: // Left stick Y
				gamepad->axis[1] = cookie;
				break;
			case kHIDUsage_GD_Z: // Left trigger
				gamepad->axis[4] = cookie;
				break;
			case kHIDUsage_GD_Rx: // Right stick X
				gamepad->axis[2] = cookie;
				break;
			case kHIDUsage_GD_Ry: // Right stick Y
				gamepad->axis[3] = cookie;
				break;
			case kHIDUsage_GD_Rz: // Right trigger
				gamepad->axis[5] = cookie;
				break;
			case kHIDUsage_GD_Hatswitch:
				break;
			default:
				break;
			}
			break;
		case kHIDPage_Button:
			if ((usage >= 1) && (usage <= 15)) {
				// Button 1-11
				gamepad->buttons[usage - 1] = cookie;
			}
			break;
		default:
			break;
		}

		if (elemType == kIOHIDElementTypeInput_Misc || elemType == kIOHIDElementTypeInput_Button || elemType == kIOHIDElementTypeInput_Axis) {
			if (!IOHIDQueueContainsElement(gamepad->hidQueueRef, elementRef))
				IOHIDQueueAddElement(gamepad->hidQueueRef, elementRef);
		}
	}
}

void HIDGamepad_unbind(struct HIDGamepad *gamepad) {
	kinc_log(KINC_LOG_LEVEL_INFO, "HIDGamepad.unbind: idx:%d [0x%x:0x%x] [%s] [%s]", gamepad->padIndex, gamepad->hidDeviceVendorID, gamepad->hidDeviceProductID,
	         gamepad->hidDeviceVendor, gamepad->hidDeviceProduct);

	if (gamepad->hidQueueRef) {
		IOHIDQueueStop(gamepad->hidQueueRef);
		IOHIDQueueUnscheduleFromRunLoop(gamepad->hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	if (gamepad->hidDeviceRef) {
		IOHIDDeviceUnscheduleFromRunLoop(gamepad->hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDDeviceClose(gamepad->hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);
	}

	if (gamepad->padIndex >= 0) {
	}

	reset(gamepad);
}

static void reset(struct HIDGamepad *gamepad) {
	gamepad->padIndex = -1;
	gamepad->hidDeviceRef = NULL;
	gamepad->hidQueueRef = NULL;
	gamepad->hidDeviceVendor[0] = '\0';
	gamepad->hidDeviceProduct[0] = '\0';
	gamepad->hidDeviceVendorID = 0;
	gamepad->hidDeviceProductID = 0;

	memset(gamepad->axis, 0, sizeof(gamepad->axis));
	memset(gamepad->buttons, 0, sizeof(gamepad->buttons));
}

static void buttonChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize button value to the range [0.0, 1.0] (0 - release, 1 - pressed)
	double min = IOHIDElementGetLogicalMin(elementRef);
	double max = IOHIDElementGetLogicalMax(elementRef);
	double normalize = (rawValue - min) / (max - min);

	kinc_internal_gamepad_trigger_button(gamepad->padIndex, buttonIndex, normalize);

	if (debugButtonInput)
		logButton(buttonIndex, (normalize != 0));
}

static void axisChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize axis value to the range [-1.0, 1.0] (e.g. -1 - left, 0 - release, 1 - right)
	double min = IOHIDElementGetPhysicalMin(elementRef);
	double max = IOHIDElementGetPhysicalMax(elementRef);
	double normalize = normalize = (((rawValue - min) / (max - min)) * 2) - 1;

	// Invert Y axis
	if (axisIndex % 2 == 1)
		normalize = -normalize;

	kinc_internal_gamepad_trigger_axis(gamepad->padIndex, axisIndex, normalize);

	if (debugAxisInput)
		logAxis(axisIndex);
}

static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef) {}

static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender) {
	struct HIDGamepad *pad = (struct HIDGamepad *)inContext;
	do {
		IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout((IOHIDQueueRef)inSender, 0.);
		if (!valueRef)
			break;

		// process the HID value reference
		IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);

		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);

		// Check button
		for (int i = 0, c = sizeof(pad->buttons); i < c; ++i) {
			if (cookie == pad->buttons[i]) {
				buttonChanged(pad, elementRef, valueRef, i);
				break;
			}
		}

		// Check axes
		for (int i = 0, c = sizeof(pad->axis); i < c; ++i) {
			if (cookie == pad->axis[i]) {
				axisChanged(pad, elementRef, valueRef, i);
				break;
			}
		}

		CFRelease(valueRef);
	} while (1);
}

const char *kinc_gamepad_vendor(int gamepad) {
	return "unknown";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "unknown";
}
