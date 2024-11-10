#include <kinc/backend/HIDManager.h>
#include <kinc/log.h>

static int initHIDManager(struct HIDManager *manager);
static bool addMatchingArray(struct HIDManager *manager, CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef);
static CFMutableDictionaryRef createDeviceMatchingDictionary(struct HIDManager *manager, uint32_t inUsagePage, uint32_t inUsage);

static void deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);
static void deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);

void HIDManager_init(struct HIDManager *manager) {
	manager->managerRef = 0x0;
	initHIDManager(manager);
}

void HIDManager_destroy(struct HIDManager *manager) {
	if (manager->managerRef) {
		IOHIDManagerUnscheduleFromRunLoop(manager->managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDManagerClose(manager->managerRef, kIOHIDOptionsTypeNone);
	}
}

static int initHIDManager(struct HIDManager *manager) {
	// Initialize the IOHIDManager
	manager->managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(manager->managerRef) == IOHIDManagerGetTypeID()) {

		// Create a matching dictionary for gamepads and joysticks
		CFMutableArrayRef matchingCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (matchingCFArrayRef) {
			// Create a device matching dictionary for joysticks
			CFDictionaryRef matchingCFDictRef = createDeviceMatchingDictionary(manager, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
			addMatchingArray(manager, matchingCFArrayRef, matchingCFDictRef);

			// Create a device matching dictionary for game pads
			matchingCFDictRef = createDeviceMatchingDictionary(manager, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
			addMatchingArray(manager, matchingCFArrayRef, matchingCFDictRef);
		}
		else {
			kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFArrayCreateMutable failed.", __PRETTY_FUNCTION__);
			return -1;
		}

		// Set the HID device matching array
		IOHIDManagerSetDeviceMatchingMultiple(manager->managerRef, matchingCFArrayRef);
		CFRelease(matchingCFArrayRef);

		// Open manager
		IOHIDManagerOpen(manager->managerRef, kIOHIDOptionsTypeNone);

		// Register routines to be called when (matching) devices are connected or disconnected
		IOHIDManagerRegisterDeviceMatchingCallback(manager->managerRef, deviceConnected, manager);
		IOHIDManagerRegisterDeviceRemovalCallback(manager->managerRef, deviceRemoved, manager);

		IOHIDManagerScheduleWithRunLoop(manager->managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

		return 0;
	}
	return -1;
}

bool addMatchingArray(struct HIDManager *manager, CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef) {
	if (matchingCFDictRef) {
		// Add it to the matching array
		CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
		CFRelease(matchingCFDictRef); // and release it
		return true;
	}
	return false;
}

CFMutableDictionaryRef createDeviceMatchingDictionary(struct HIDManager *manager, uint32_t inUsagePage, uint32_t inUsage) {
	// Create a dictionary to add usage page/usages to
	CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (result) {
		if (inUsagePage) {
			// Add key for device type to refine the matching dictionary.
			CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
			if (pageCFNumberRef) {
				CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
				CFRelease(pageCFNumberRef);

				// note: the usage is only valid if the usage page is also defined
				if (inUsage) {
					CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
					if (usageCFNumberRef) {
						CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
						CFRelease(usageCFNumberRef);
					}
					else {
						kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
					}
				}
			}
			else {
				kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
			}
		}
	}
	else {
		kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
	}
	return result;
}

// HID device plugged callback
void deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	// Reference manager
	struct HIDManager *manager = (struct HIDManager *)inContext;

	// Find an empty slot in the devices list and add the new device there
	// TODO: does this need to be made thread safe?
	struct HIDManagerDeviceRecord *device = &manager->devices[0];
	for (int i = 0; i < KINC_MAX_HID_DEVICES; ++i, ++device) {
		if (!device->connected) {
			device->connected = true;
			device->device = inIOHIDDeviceRef;
			HIDGamepad_bind(&device->pad, inIOHIDDeviceRef, i);
			break;
		}
	}
}

// HID device unplugged callback
void deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	// Reference manager
	struct HIDManager *manager = (struct HIDManager *)inContext;

	// TODO: does this need to be made thread safe?
	struct HIDManagerDeviceRecord *device = &manager->devices[0];
	for (int i = 0; i < KINC_MAX_HID_DEVICES; ++i, ++device) {
		// TODO: is comparing IOHIDDeviceRef to match devices safe? Is there a better way?
		if (device->connected && device->device == inIOHIDDeviceRef) {
			device->connected = false;
			device->device = NULL;
			HIDGamepad_unbind(&device->pad);
			break;
		}
	}
}
