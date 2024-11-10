#include "gamepad.h"

#include <kinc/input/gamepad.h>

#include <fcntl.h>
#include <libudev.h>
#include <linux/joystick.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct HIDGamepad {
	int idx;
	char gamepad_dev_name[256];
	char name[385];
	int file_descriptor;
	bool connected;
	struct js_event gamepadEvent;
};

static void HIDGamepad_open(struct HIDGamepad *pad) {
	pad->file_descriptor = open(pad->gamepad_dev_name, O_RDONLY | O_NONBLOCK);
	if (pad->file_descriptor < 0) {
		pad->connected = false;
	}
	else {
		pad->connected = true;

		char buf[128];
		if (ioctl(pad->file_descriptor, JSIOCGNAME(sizeof(buf)), buf) < 0) {
			strncpy(buf, "Unknown", sizeof(buf));
		}
		pad->name[0] = 0;
		// snprintf(pad->name, sizeof(pad->name), "%s(%s)", buf, pad->gamepad_dev_name); // TODO: valgrind error
		kinc_internal_gamepad_trigger_connect(pad->idx);
	}
}

static void HIDGamepad_init(struct HIDGamepad *pad, int index) {
	pad->file_descriptor = -1;
	pad->connected = false;
	pad->gamepad_dev_name[0] = 0;
	if (index >= 0 && index < 12) {
		pad->idx = index;
		snprintf(pad->gamepad_dev_name, sizeof(pad->gamepad_dev_name), "/dev/input/js%d", pad->idx);
		HIDGamepad_open(pad);
	}
}

static void HIDGamepad_close(struct HIDGamepad *pad) {
	if (pad->connected) {
		kinc_internal_gamepad_trigger_disconnect(pad->idx);
		close(pad->file_descriptor);
		pad->file_descriptor = -1;
		pad->connected = false;
	}
}

void HIDGamepad_processEvent(struct HIDGamepad *pad, struct js_event e) {
	switch (e.type) {
	case JS_EVENT_BUTTON:
		kinc_internal_gamepad_trigger_button(pad->idx, e.number, e.value);
		break;
	case JS_EVENT_AXIS: {
		float value = e.number % 2 == 0 ? e.value : -e.value;
		kinc_internal_gamepad_trigger_axis(pad->idx, e.number, value / 32767.0f);
		break;
	}
	default:
		break;
	}
}

void HIDGamepad_update(struct HIDGamepad *pad) {
	if (pad->connected) {
		while (read(pad->file_descriptor, &pad->gamepadEvent, sizeof(pad->gamepadEvent)) > 0) {
			HIDGamepad_processEvent(pad, pad->gamepadEvent);
		}
	}
}

struct HIDGamepadUdevHelper {
	struct udev *udevPtr;
	struct udev_monitor *udevMonitorPtr;
	int udevMonitorFD;
};

static struct HIDGamepadUdevHelper udev_helper;

static struct HIDGamepad gamepads[KINC_GAMEPAD_MAX_COUNT];

static void HIDGamepadUdevHelper_openOrCloseGamepad(struct HIDGamepadUdevHelper *helper, struct udev_device *dev) {
	const char *action = udev_device_get_action(dev);
	if (!action)
		action = "add";

	const char *joystickDevnodeName = strstr(udev_device_get_devnode(dev), "js");

	if (joystickDevnodeName) {
		int joystickDevnodeIndex;
		sscanf(joystickDevnodeName, "js%d", &joystickDevnodeIndex);

		if (!strcmp(action, "add")) {
			HIDGamepad_open(&gamepads[joystickDevnodeIndex]);
		}

		if (!strcmp(action, "remove")) {
			HIDGamepad_close(&gamepads[joystickDevnodeIndex]);
		}
	}
}

static void HIDGamepadUdevHelper_processDevice(struct HIDGamepadUdevHelper *helper, struct udev_device *dev) {
	if (dev) {
		if (udev_device_get_devnode(dev))
			HIDGamepadUdevHelper_openOrCloseGamepad(helper, dev);

		udev_device_unref(dev);
	}
}

static void HIDGamepadUdevHelper_init(struct HIDGamepadUdevHelper *helper) {
	struct udev *udevPtrNew = udev_new();

	// enumerate
	struct udev_enumerate *enumerate = udev_enumerate_new(udevPtrNew);

	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
	struct udev_list_entry *entry;

	udev_list_entry_foreach(entry, devices) {
		const char *path = udev_list_entry_get_name(entry);
		struct udev_device *dev = udev_device_new_from_syspath(udevPtrNew, path);
		HIDGamepadUdevHelper_processDevice(helper, dev);
	}

	udev_enumerate_unref(enumerate);

	// setup mon
	helper->udevMonitorPtr = udev_monitor_new_from_netlink(udevPtrNew, "udev");

	udev_monitor_filter_add_match_subsystem_devtype(helper->udevMonitorPtr, "input", NULL);
	udev_monitor_enable_receiving(helper->udevMonitorPtr);

	helper->udevMonitorFD = udev_monitor_get_fd(helper->udevMonitorPtr);

	helper->udevPtr = udevPtrNew;
}

static void HIDGamepadUdevHelper_update(struct HIDGamepadUdevHelper *helper) {
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(helper->udevMonitorFD, &fds);

	if (FD_ISSET(helper->udevMonitorFD, &fds)) {
		struct udev_device *dev = udev_monitor_receive_device(helper->udevMonitorPtr);
		HIDGamepadUdevHelper_processDevice(helper, dev);
	}
}

static void HIDGamepadUdevHelper_close(struct HIDGamepadUdevHelper *helper) {
	udev_unref(helper->udevPtr);
}

void kinc_linux_initHIDGamepads() {
	for (int i = 0; i < KINC_GAMEPAD_MAX_COUNT; ++i) {
		HIDGamepad_init(&gamepads[i], i);
	}
	HIDGamepadUdevHelper_init(&udev_helper);
}

void kinc_linux_updateHIDGamepads() {
	HIDGamepadUdevHelper_update(&udev_helper);
	for (int i = 0; i < KINC_GAMEPAD_MAX_COUNT; ++i) {
		HIDGamepad_update(&gamepads[i]);
	}
}

void kinc_linux_closeHIDGamepads() {
	HIDGamepadUdevHelper_close(&udev_helper);
}

const char *kinc_gamepad_vendor(int gamepad) {
	return "Linux gamepad";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return gamepad >= 0 && gamepad < KINC_GAMEPAD_MAX_COUNT ? gamepads[gamepad].name : "";
}

bool kinc_gamepad_connected(int gamepad) {
	return gamepad >= 0 && gamepad < KINC_GAMEPAD_MAX_COUNT && gamepads[gamepad].connected;
}

void kinc_gamepad_rumble(int gamepad, float left, float right) {}
