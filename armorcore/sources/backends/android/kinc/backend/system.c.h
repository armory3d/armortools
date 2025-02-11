#define EGL_NO_PLATFORM_SPECIFIC_TYPES
#include <EGL/egl.h>
#include <kinc/error.h>
#include <kinc/backend/Android.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/gamepad.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <android/sensor.h>
#include <android/window.h>
#include <android_native_app_glue.h>
#include <kinc/input/pen.h>
#include <kinc/input/surface.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/threads/mutex.h>
#include <kinc/video.h>
#include <kinc/window.h>
#include <unistd.h>
#include <stdlib.h>

void pauseAudio();
void resumeAudio();

static struct android_app *app = NULL;
static ANativeActivity *activity = NULL;
static ASensorManager *sensorManager = NULL;
static const ASensor *accelerometerSensor = NULL;
static const ASensor *gyroSensor = NULL;
static ASensorEventQueue *sensorEventQueue = NULL;

static bool started = false;
static bool paused = true;
static bool displayIsInitialized = false;
static bool appIsForeground = false;
static bool activityJustResized = false;

#include <assert.h>
#include <kinc/log.h>

#include <vulkan/vulkan_android.h>
#include <vulkan/vulkan_core.h>

VkResult kinc_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface) {
	assert(app->window != NULL);
	VkAndroidSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.window = app->window;
	return vkCreateAndroidSurfaceKHR(instance, &createInfo, NULL, surface);
}

void kinc_vulkan_get_instance_extensions(const char **names, int *index, int max) {
	assert(*index + 1 < max);
	names[(*index)++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
}

VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_android_surface.html#_issues
	//
	// 1) Does Android need a way to query for compatibility between a particular physical device (and queue family?)
	// and a specific Android display?
	// RESOLVED: No. Currently on Android, any physical device is expected to be able to present to the system compositor,
	// and all queue families must support the necessary image layout transitions and synchronization operations.
	return true;
}

void kinc_vulkan_init_window(int window);

static void initDisplay() {
	kinc_vulkan_init_window(0);
}

static void termDisplay() {
}

static void updateAppForegroundStatus(bool displayIsInitializedValue, bool appIsForegroundValue) {
	bool oldStatus = displayIsInitialized && appIsForeground;
	displayIsInitialized = displayIsInitializedValue;
	appIsForeground = appIsForegroundValue;
	bool newStatus = displayIsInitialized && appIsForeground;
	if (oldStatus != newStatus) {
		if (newStatus) {
			kinc_internal_foreground_callback();
		}
		else {
			kinc_internal_background_callback();
		}
	}
}

static bool isGamepadEvent(AInputEvent *event) {
	return ((AInputEvent_getSource(event) & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD ||
	        (AInputEvent_getSource(event) & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK ||
	        (AInputEvent_getSource(event) & AINPUT_SOURCE_DPAD) == AINPUT_SOURCE_DPAD);
}

static bool isPenEvent(AInputEvent *event) {
	return (AInputEvent_getSource(event) & AINPUT_SOURCE_STYLUS) == AINPUT_SOURCE_STYLUS;
}

static void touchInput(AInputEvent *event) {
	int action = AMotionEvent_getAction(event);
	int index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
	int id = AMotionEvent_getPointerId(event, index);
	float x = AMotionEvent_getX(event, index);
	float y = AMotionEvent_getY(event, index);
	switch (action & AMOTION_EVENT_ACTION_MASK) {
	case AMOTION_EVENT_ACTION_DOWN:
	case AMOTION_EVENT_ACTION_POINTER_DOWN:
		if (id == 0) {
			kinc_internal_mouse_trigger_press(0, 0, x, y);
		}
		if (isPenEvent(event)) {
			kinc_internal_pen_trigger_press(0, x, y, AMotionEvent_getPressure(event, index));
		}
		kinc_internal_surface_trigger_touch_start(id, x, y);
		break;
	case AMOTION_EVENT_ACTION_MOVE:
	case AMOTION_EVENT_ACTION_HOVER_MOVE: {
		size_t count = AMotionEvent_getPointerCount(event);
		for (int i = 0; i < count; ++i) {
			id = AMotionEvent_getPointerId(event, i);
			x = AMotionEvent_getX(event, i);
			y = AMotionEvent_getY(event, i);
			if (id == 0) {
				kinc_internal_mouse_trigger_move(0, x, y);
			}
			if (isPenEvent(event)) {
				kinc_internal_pen_trigger_move(0, x, y, AMotionEvent_getPressure(event, index));
			}
			kinc_internal_surface_trigger_move(id, x, y);
		}
	} break;
	case AMOTION_EVENT_ACTION_UP:
	case AMOTION_EVENT_ACTION_CANCEL:
	case AMOTION_EVENT_ACTION_POINTER_UP:
		if (id == 0) {
			kinc_internal_mouse_trigger_release(0, 0, x, y);
		}
		if (isPenEvent(event)) {
			kinc_internal_pen_trigger_release(0, x, y, AMotionEvent_getPressure(event, index));
		}
		kinc_internal_surface_trigger_touch_end(id, x, y);
		break;
	case AMOTION_EVENT_ACTION_SCROLL:
		if (id == 0) {
			float scroll = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);
			kinc_internal_mouse_trigger_scroll(0, -(int)scroll);
		}
		break;
	}
}

static float last_x = 0.0f;
static float last_y = 0.0f;
static float last_l = 0.0f;
static float last_r = 0.0f;
static bool last_hat_left = false;
static bool last_hat_right = false;
static bool last_hat_up = false;
static bool last_hat_down = false;

static int32_t input(struct android_app *app, AInputEvent *event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		int source = AInputEvent_getSource(event);
		if (((source & AINPUT_SOURCE_TOUCHSCREEN) == AINPUT_SOURCE_TOUCHSCREEN) || ((source & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE)) {
			touchInput(event);
			return 1;
		}
		else if ((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK) {
			// int id = AInputEvent_getDeviceId(event);

			float x = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0);
			if (x != last_x) {
				kinc_internal_gamepad_trigger_axis(0, 0, x);
				last_x = x;
			}

			float y = -AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0);
			if (y != last_y) {
				kinc_internal_gamepad_trigger_axis(0, 1, y);
				last_y = y;
			}

			float l = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_LTRIGGER, 0);
			if (l != last_l) {
				kinc_internal_gamepad_trigger_button(0, 6, l);
				last_l = l;
			}

			float r = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RTRIGGER, 0);
			if (r != last_r) {
				kinc_internal_gamepad_trigger_button(0, 7, r);
				last_r = r;
			}

			float hat_x = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, 0);

			bool hat_left = false;
			bool hat_right = false;
			if (hat_x < -0.5f) {
				hat_left = true;
			}
			else if (hat_x > 0.5f) {
				hat_right = true;
			}

			float hat_y = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, 0);

			bool hat_up = false;
			bool hat_down = false;
			if (hat_y < -0.5f) {
				hat_up = true;
			}
			else if (hat_y > 0.5f) {
				hat_down = true;
			}

			if (hat_left != last_hat_left) {
				kinc_internal_gamepad_trigger_button(0, 14, hat_left ? 1.0f : 0.0f);
				last_hat_left = hat_left;
			}

			if (hat_right != last_hat_right) {
				kinc_internal_gamepad_trigger_button(0, 15, hat_right ? 1.0f : 0.0f);
				last_hat_right = hat_right;
			}

			if (hat_up != last_hat_up) {
				kinc_internal_gamepad_trigger_button(0, 12, hat_up ? 1.0f : 0.0f);
				last_hat_up = hat_up;
			}

			if (hat_down != last_hat_down) {
				kinc_internal_gamepad_trigger_button(0, 13, hat_down ? 1.0f : 0.0f);
				last_hat_down = hat_down;
			}

			return 1;
		}
	}
	else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
		int32_t code = AKeyEvent_getKeyCode(event);

		if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
			int shift = AKeyEvent_getMetaState(event) & AMETA_SHIFT_ON;
			if (shift) {
				switch (code) {
				case AKEYCODE_1:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_EXCLAMATION);
					kinc_internal_keyboard_trigger_key_press('!');
					return 1;
				case AKEYCODE_4:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_DOLLAR);
					kinc_internal_keyboard_trigger_key_press('$');
					return 1;
				case AKEYCODE_5:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_PERCENT);
					kinc_internal_keyboard_trigger_key_press('%');
					return 1;
				case AKEYCODE_6:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_CIRCUMFLEX);
					kinc_internal_keyboard_trigger_key_press('^');
					return 1;
				case AKEYCODE_7:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_AMPERSAND);
					kinc_internal_keyboard_trigger_key_press('&');
					return 1;
				case AKEYCODE_9:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_OPEN_PAREN);
					kinc_internal_keyboard_trigger_key_press('(');
					return 1;
				case AKEYCODE_0:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_CLOSE_PAREN);
					kinc_internal_keyboard_trigger_key_press(')');
					return 1;
				case AKEYCODE_COMMA:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_LESS_THAN);
					kinc_internal_keyboard_trigger_key_press('<');
					return 1;
				case AKEYCODE_PERIOD:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_GREATER_THAN);
					kinc_internal_keyboard_trigger_key_press('>');
					return 1;
				case AKEYCODE_MINUS:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_UNDERSCORE);
					kinc_internal_keyboard_trigger_key_press('_');
					return 1;
				case AKEYCODE_SLASH:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_QUESTIONMARK);
					kinc_internal_keyboard_trigger_key_press('?');
					return 1;
				case AKEYCODE_BACKSLASH:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_PIPE);
					kinc_internal_keyboard_trigger_key_press('|');
					return 1;
				case AKEYCODE_LEFT_BRACKET:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_OPEN_CURLY_BRACKET);
					kinc_internal_keyboard_trigger_key_press('{');
					return 1;
				case AKEYCODE_RIGHT_BRACKET:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_CLOSE_CURLY_BRACKET);
					kinc_internal_keyboard_trigger_key_press('}');
					return 1;
				case AKEYCODE_SEMICOLON:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_COLON);
					kinc_internal_keyboard_trigger_key_press(':');
					return 1;
				case AKEYCODE_APOSTROPHE:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_DOUBLE_QUOTE);
					kinc_internal_keyboard_trigger_key_press('"');
					return 1;
				case AKEYCODE_GRAVE:
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_TILDE);
					kinc_internal_keyboard_trigger_key_press('~');
					return 1;
				}
			}
			switch (code) {
			case AKEYCODE_SHIFT_LEFT:
			case AKEYCODE_SHIFT_RIGHT:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SHIFT);
				return 1;
			case AKEYCODE_DEL:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACKSPACE);
				return 1;
			case AKEYCODE_ENTER:
			case AKEYCODE_NUMPAD_ENTER:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_RETURN);
				return 1;
			case AKEYCODE_DPAD_CENTER:
			case AKEYCODE_BUTTON_B:
				kinc_internal_gamepad_trigger_button(0, 1, 1);
				return 1;
			case AKEYCODE_BACK:
				if (AKeyEvent_getMetaState(event) & AMETA_ALT_ON) { // Xperia Play
					kinc_internal_gamepad_trigger_button(0, 1, 1);
					return 1;
				}
				else {
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACK);
					return 1;
				}
			case AKEYCODE_BUTTON_A:
				kinc_internal_gamepad_trigger_button(0, 0, 1);
				return 1;
			case AKEYCODE_BUTTON_Y:
				kinc_internal_gamepad_trigger_button(0, 3, 1);
				return 1;
			case AKEYCODE_BUTTON_X:
				kinc_internal_gamepad_trigger_button(0, 2, 1);
				return 1;
			case AKEYCODE_BUTTON_L1:
				kinc_internal_gamepad_trigger_button(0, 4, 1);
				return 1;
			case AKEYCODE_BUTTON_R1:
				kinc_internal_gamepad_trigger_button(0, 5, 1);
				return 1;
			case AKEYCODE_BUTTON_L2:
				kinc_internal_gamepad_trigger_button(0, 6, 1);
				return 1;
			case AKEYCODE_BUTTON_R2:
				kinc_internal_gamepad_trigger_button(0, 7, 1);
				return 1;
			case AKEYCODE_BUTTON_SELECT:
				kinc_internal_gamepad_trigger_button(0, 8, 1);
				return 1;
			case AKEYCODE_BUTTON_START:
				kinc_internal_gamepad_trigger_button(0, 9, 1);
				return 1;
			case AKEYCODE_BUTTON_THUMBL:
				kinc_internal_gamepad_trigger_button(0, 10, 1);
				return 1;
			case AKEYCODE_BUTTON_THUMBR:
				kinc_internal_gamepad_trigger_button(0, 11, 1);
				return 1;
			case AKEYCODE_DPAD_UP:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 12, 1);
				else
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_UP);
				return 1;
			case AKEYCODE_DPAD_DOWN:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 13, 1);
				else
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_DOWN);
				return 1;
			case AKEYCODE_DPAD_LEFT:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 14, 1);
				else
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_LEFT);
				return 1;
			case AKEYCODE_DPAD_RIGHT:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 15, 1);
				else
					kinc_internal_keyboard_trigger_key_down(KINC_KEY_RIGHT);
				return 1;
			case AKEYCODE_BUTTON_MODE:
				kinc_internal_gamepad_trigger_button(0, 16, 1);
				return 1;
			case AKEYCODE_STAR:
			case AKEYCODE_NUMPAD_MULTIPLY:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_MULTIPLY);
				kinc_internal_keyboard_trigger_key_press('*');
				return 1;
			case AKEYCODE_POUND:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_HASH);
				kinc_internal_keyboard_trigger_key_press('#');
				return 1;
			case AKEYCODE_COMMA:
			case AKEYCODE_NUMPAD_COMMA:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_COMMA);
				kinc_internal_keyboard_trigger_key_press(',');
				return 1;
			case AKEYCODE_PERIOD:
			case AKEYCODE_NUMPAD_DOT:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_PERIOD);
				kinc_internal_keyboard_trigger_key_press('.');
				return 1;
			case AKEYCODE_SPACE:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SPACE);
				kinc_internal_keyboard_trigger_key_press(' ');
				return 1;
			case AKEYCODE_MINUS:
			case AKEYCODE_NUMPAD_SUBTRACT:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_HYPHEN_MINUS);
				kinc_internal_keyboard_trigger_key_press('-');
				return 1;
			case AKEYCODE_EQUALS:
			case AKEYCODE_NUMPAD_EQUALS:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_EQUALS);
				kinc_internal_keyboard_trigger_key_press('=');
				return 1;
			case AKEYCODE_LEFT_BRACKET:
			case AKEYCODE_NUMPAD_LEFT_PAREN:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_OPEN_BRACKET);
				kinc_internal_keyboard_trigger_key_press('[');
				return 1;
			case AKEYCODE_RIGHT_BRACKET:
			case AKEYCODE_NUMPAD_RIGHT_PAREN:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_CLOSE_BRACKET);
				kinc_internal_keyboard_trigger_key_press(']');
				return 1;
			case AKEYCODE_BACKSLASH:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACK_SLASH);
				kinc_internal_keyboard_trigger_key_press('\\');
				return 1;
			case AKEYCODE_SEMICOLON:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SEMICOLON);
				kinc_internal_keyboard_trigger_key_press(';');
				return 1;
			case AKEYCODE_APOSTROPHE:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_QUOTE);
				kinc_internal_keyboard_trigger_key_press('\'');
				return 1;
			case AKEYCODE_GRAVE:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACK_QUOTE);
				kinc_internal_keyboard_trigger_key_press('`');
				return 1;
			case AKEYCODE_SLASH:
			case AKEYCODE_NUMPAD_DIVIDE:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SLASH);
				kinc_internal_keyboard_trigger_key_press('/');
				return 1;
			case AKEYCODE_AT:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_AT);
				kinc_internal_keyboard_trigger_key_press('@');
				return 1;
			case AKEYCODE_PLUS:
			case AKEYCODE_NUMPAD_ADD:
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_PLUS);
				kinc_internal_keyboard_trigger_key_press('+');
				return 1;
			// (DK) Amazon FireTV remote/controller mappings
			// (DK) TODO handle multiple pads (up to 4 possible)
			case AKEYCODE_MENU:
				kinc_internal_gamepad_trigger_button(0, 9, 1);
				return 1;
			case AKEYCODE_MEDIA_REWIND:
				kinc_internal_gamepad_trigger_button(0, 10, 1);
				return 1;
			case AKEYCODE_MEDIA_FAST_FORWARD:
				kinc_internal_gamepad_trigger_button(0, 11, 1);
				return 1;
			case AKEYCODE_MEDIA_PLAY_PAUSE:
				kinc_internal_gamepad_trigger_button(0, 12, 1);
				return 1;
			// (DK) /Amazon FireTV remote/controller mappings
			default:
				if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9) {
					kinc_internal_keyboard_trigger_key_down(code + KINC_KEY_NUMPAD_0 - AKEYCODE_NUMPAD_0);
					kinc_internal_keyboard_trigger_key_press(code + KINC_KEY_NUMPAD_0 - AKEYCODE_NUMPAD_0);
					return 1;
				}
				else if (code >= AKEYCODE_0 && code <= AKEYCODE_9) {
					kinc_internal_keyboard_trigger_key_down(code + KINC_KEY_0 - AKEYCODE_0);
					kinc_internal_keyboard_trigger_key_press(code + KINC_KEY_0 - AKEYCODE_0);
					return 1;
				}
				else if (code >= AKEYCODE_A && code <= AKEYCODE_Z) {
					kinc_internal_keyboard_trigger_key_down(code + KINC_KEY_A - AKEYCODE_A);
					kinc_internal_keyboard_trigger_key_press(code + (shift ? 'A' : 'a') - AKEYCODE_A);
					return 1;
				}
			}
		}
		else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
			int shift = AKeyEvent_getMetaState(event) & AMETA_SHIFT_ON;
			if (shift) {
				switch (code) {
				case AKEYCODE_1:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_EXCLAMATION);
					return 1;
				case AKEYCODE_4:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_DOLLAR);
					return 1;
				case AKEYCODE_5:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_PERCENT);
					return 1;
				case AKEYCODE_6:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_CIRCUMFLEX);
					return 1;
				case AKEYCODE_7:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_AMPERSAND);
					return 1;
				case AKEYCODE_9:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_OPEN_PAREN);
					return 1;
				case AKEYCODE_0:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_CLOSE_PAREN);
					return 1;
				case AKEYCODE_COMMA:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_LESS_THAN);
					return 1;
				case AKEYCODE_PERIOD:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_GREATER_THAN);
					return 1;
				case AKEYCODE_MINUS:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_UNDERSCORE);
					return 1;
				case AKEYCODE_SLASH:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_QUESTIONMARK);
					return 1;
				case AKEYCODE_BACKSLASH:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_PIPE);
					return 1;
				case AKEYCODE_LEFT_BRACKET:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_OPEN_CURLY_BRACKET);
					return 1;
				case AKEYCODE_RIGHT_BRACKET:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_CLOSE_CURLY_BRACKET);
					return 1;
				case AKEYCODE_SEMICOLON:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_COLON);
					return 1;
				case AKEYCODE_APOSTROPHE:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_DOUBLE_QUOTE);
					return 1;
				case AKEYCODE_GRAVE:
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_TILDE);
					return 1;
				}
			}
			switch (code) {
			case AKEYCODE_SHIFT_LEFT:
			case AKEYCODE_SHIFT_RIGHT:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SHIFT);
				return 1;
			case AKEYCODE_DEL:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACKSPACE);
				return 1;
			case AKEYCODE_ENTER:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_RETURN);
				return 1;
			case AKEYCODE_DPAD_CENTER:
			case AKEYCODE_BUTTON_B:
				kinc_internal_gamepad_trigger_button(0, 1, 0);
				return 1;
			case AKEYCODE_BACK:
				if (AKeyEvent_getMetaState(event) & AMETA_ALT_ON) { // Xperia Play
					kinc_internal_gamepad_trigger_button(0, 1, 0);
					return 1;
				}
				else {
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACK);
					return 1;
				}
			case AKEYCODE_BUTTON_A:
				kinc_internal_gamepad_trigger_button(0, 0, 0);
				return 1;
			case AKEYCODE_BUTTON_Y:
				kinc_internal_gamepad_trigger_button(0, 3, 0);
				return 1;
			case AKEYCODE_BUTTON_X:
				kinc_internal_gamepad_trigger_button(0, 2, 0);
				return 1;
			case AKEYCODE_BUTTON_L1:
				kinc_internal_gamepad_trigger_button(0, 4, 0);
				return 1;
			case AKEYCODE_BUTTON_R1:
				kinc_internal_gamepad_trigger_button(0, 5, 0);
				return 1;
			case AKEYCODE_BUTTON_L2:
				kinc_internal_gamepad_trigger_button(0, 6, 0);
				return 1;
			case AKEYCODE_BUTTON_R2:
				kinc_internal_gamepad_trigger_button(0, 7, 0);
				return 1;
			case AKEYCODE_BUTTON_SELECT:
				kinc_internal_gamepad_trigger_button(0, 8, 0);
				return 1;
			case AKEYCODE_BUTTON_START:
				kinc_internal_gamepad_trigger_button(0, 9, 0);
				return 1;
			case AKEYCODE_BUTTON_THUMBL:
				kinc_internal_gamepad_trigger_button(0, 10, 0);
				return 1;
			case AKEYCODE_BUTTON_THUMBR:
				kinc_internal_gamepad_trigger_button(0, 11, 0);
				return 1;
			case AKEYCODE_DPAD_UP:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 12, 0);
				else
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_UP);
				return 1;
			case AKEYCODE_DPAD_DOWN:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 13, 0);
				else
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_DOWN);
				return 1;
			case AKEYCODE_DPAD_LEFT:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 14, 0);
				else
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_LEFT);
				return 1;
			case AKEYCODE_DPAD_RIGHT:
				if (isGamepadEvent(event))
					kinc_internal_gamepad_trigger_button(0, 15, 0);
				else
					kinc_internal_keyboard_trigger_key_up(KINC_KEY_RIGHT);
				return 1;
			case AKEYCODE_BUTTON_MODE:
				kinc_internal_gamepad_trigger_button(0, 16, 0);
				return 1;
			case AKEYCODE_STAR:
			case AKEYCODE_NUMPAD_MULTIPLY:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_MULTIPLY);
				return 1;
			case AKEYCODE_POUND:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_HASH);
				return 1;
			case AKEYCODE_COMMA:
			case AKEYCODE_NUMPAD_COMMA:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_COMMA);
				return 1;
			case AKEYCODE_PERIOD:
			case AKEYCODE_NUMPAD_DOT:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_PERIOD);
				return 1;
			case AKEYCODE_SPACE:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SPACE);
				return 1;
			case AKEYCODE_MINUS:
			case AKEYCODE_NUMPAD_SUBTRACT:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_HYPHEN_MINUS);
				return 1;
			case AKEYCODE_EQUALS:
			case AKEYCODE_NUMPAD_EQUALS:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_EQUALS);
				return 1;
			case AKEYCODE_LEFT_BRACKET:
			case AKEYCODE_NUMPAD_LEFT_PAREN:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_OPEN_BRACKET);
				return 1;
			case AKEYCODE_RIGHT_BRACKET:
			case AKEYCODE_NUMPAD_RIGHT_PAREN:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_CLOSE_BRACKET);
				return 1;
			case AKEYCODE_BACKSLASH:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACK_SLASH);
				return 1;
			case AKEYCODE_SEMICOLON:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SEMICOLON);
				return 1;
			case AKEYCODE_APOSTROPHE:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_QUOTE);
				return 1;
			case AKEYCODE_GRAVE:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACK_QUOTE);
				return 1;
			case AKEYCODE_SLASH:
			case AKEYCODE_NUMPAD_DIVIDE:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SLASH);
				return 1;
			case AKEYCODE_AT:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_AT);
				return 1;
			case AKEYCODE_PLUS:
			case AKEYCODE_NUMPAD_ADD:
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_PLUS);
				return 1;
			// (DK) Amazon FireTV remote/controller mappings
			// (DK) TODO handle multiple pads (up to 4 possible)
			case AKEYCODE_MENU:
				kinc_internal_gamepad_trigger_button(0, 9, 0);
				return 1;
			case AKEYCODE_MEDIA_REWIND:
				kinc_internal_gamepad_trigger_button(0, 10, 0);
				return 1;
			case AKEYCODE_MEDIA_FAST_FORWARD:
				kinc_internal_gamepad_trigger_button(0, 11, 0);
				return 1;
			case AKEYCODE_MEDIA_PLAY_PAUSE:
				kinc_internal_gamepad_trigger_button(0, 12, 0);
				return 1;
			// (DK) /Amazon FireTV remote/controller mappings
			default:
				if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9) {
					kinc_internal_keyboard_trigger_key_up(code + KINC_KEY_NUMPAD_0 - AKEYCODE_NUMPAD_0);
					return 1;
				}
				else if (code >= AKEYCODE_0 && code <= AKEYCODE_9) {
					kinc_internal_keyboard_trigger_key_up(code + KINC_KEY_0 - AKEYCODE_0);
					return 1;
				}
				else if (code >= AKEYCODE_A && code <= AKEYCODE_Z) {
					kinc_internal_keyboard_trigger_key_up(code + KINC_KEY_A - AKEYCODE_A);
					return 1;
				}
			}
		}
	}
	return 0;
}

static void cmd(struct android_app *app, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		break;
	case APP_CMD_INIT_WINDOW:
		if (app->window != NULL) {
			if (!started) {
				started = true;
			}
			else {
				initDisplay();
				kinc_g4_swap_buffers();
			}

			updateAppForegroundStatus(true, appIsForeground);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		termDisplay();
		updateAppForegroundStatus(false, appIsForeground);
		break;
	case APP_CMD_GAINED_FOCUS:
		if (accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
			ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, (1000L / 60) * 1000);
		}
		if (gyroSensor != NULL) {
			ASensorEventQueue_enableSensor(sensorEventQueue, gyroSensor);
			ASensorEventQueue_setEventRate(sensorEventQueue, gyroSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		if (accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(sensorEventQueue, accelerometerSensor);
		}
		if (gyroSensor != NULL) {
			ASensorEventQueue_disableSensor(sensorEventQueue, gyroSensor);
		}
		break;
	case APP_CMD_START:
		updateAppForegroundStatus(displayIsInitialized, true);
		break;
	case APP_CMD_RESUME:
		kinc_internal_resume_callback();
		resumeAudio();
		paused = false;
		break;
	case APP_CMD_PAUSE:
		kinc_internal_pause_callback();
		pauseAudio();
		paused = true;
		break;
	case APP_CMD_STOP:
		updateAppForegroundStatus(displayIsInitialized, false);
		break;
	case APP_CMD_DESTROY:
		kinc_internal_shutdown_callback();
		break;
	case APP_CMD_CONFIG_CHANGED: {

		break;
	}
	}
}

static void resize(ANativeActivity *activity, ANativeWindow *window) {
	activityJustResized = true;
}

ANativeActivity *kinc_android_get_activity(void) {
	return activity;
}

AAssetManager *kinc_android_get_asset_manager(void) {
	return activity->assetManager;
}

jclass kinc_android_find_class(JNIEnv *env, const char *name) {
	jobject nativeActivity = activity->clazz;
	jclass acl = (*env)->GetObjectClass(env, nativeActivity);
	jmethodID getClassLoader = (*env)->GetMethodID(env, acl, "getClassLoader", "()Ljava/lang/ClassLoader;");
	jobject cls = (*env)->CallObjectMethod(env, nativeActivity, getClassLoader);
	jclass classLoader = (*env)->FindClass(env, "java/lang/ClassLoader");
	jmethodID findClass = (*env)->GetMethodID(env, classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	jstring strClassName = (*env)->NewStringUTF(env, name);
	jclass clazz = (jclass)((*env)->CallObjectMethod(env, cls, findClass, strClassName));
	(*env)->DeleteLocalRef(env, strClassName);
	return clazz;
}

#define UNICODE_STACK_SIZE 256
static uint16_t unicode_stack[UNICODE_STACK_SIZE];
static int unicode_stack_index = 0;
static kinc_mutex_t unicode_mutex;

JNIEXPORT void JNICALL Java_tech_kinc_KincActivity_nativeKincKeyPress(JNIEnv *env, jobject jobj, jstring chars) {
	const jchar *text = (*env)->GetStringChars(env, chars, NULL);
	const jsize length = (*env)->GetStringLength(env, chars);

	kinc_mutex_lock(&unicode_mutex);
	for (jsize i = 0; i < length && unicode_stack_index < UNICODE_STACK_SIZE; ++i) {
		unicode_stack[unicode_stack_index++] = text[i];
	}
	kinc_mutex_unlock(&unicode_mutex);

	(*env)->ReleaseStringChars(env, chars, text);
}

void KincAndroidKeyboardInit() {
	JNIEnv *env;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);

	jclass clazz = kinc_android_find_class(env, "tech.kinc.KincActivity");

	// String chars
	JNINativeMethod methodTable[] = {{"nativeKincKeyPress", "(Ljava/lang/String;)V", (void *)Java_tech_kinc_KincActivity_nativeKincKeyPress}};

	int methodTableSize = sizeof(methodTable) / sizeof(methodTable[0]);

	int failure = (*env)->RegisterNatives(env, clazz, methodTable, methodTableSize);
	if (failure != 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Failed to register KincActivity.nativeKincKeyPress");
	}

	(*activity->vm)->DetachCurrentThread(activity->vm);
}

static bool keyboard_active = false;

void kinc_keyboard_show() {
	keyboard_active = true;
	JNIEnv *env;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	(*env)->CallStaticVoidMethod(env, koreActivityClass, (*env)->GetStaticMethodID(env, koreActivityClass, "showKeyboard", "()V"));
	(*activity->vm)->DetachCurrentThread(activity->vm);
}

void kinc_keyboard_hide() {
	keyboard_active = false;
	JNIEnv *env;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	(*env)->CallStaticVoidMethod(env, koreActivityClass, (*env)->GetStaticMethodID(env, koreActivityClass, "hideKeyboard", "()V"));
	(*activity->vm)->DetachCurrentThread(activity->vm);
}

bool kinc_keyboard_active() {
	return keyboard_active;
}

void kinc_load_url(const char *url) {
	JNIEnv *env;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	jstring jurl = (*env)->NewStringUTF(env, url);
	(*env)->CallStaticVoidMethod(env, koreActivityClass, (*env)->GetStaticMethodID(env, koreActivityClass, "loadURL", "(Ljava/lang/String;)V"), jurl);
	(*activity->vm)->DetachCurrentThread(activity->vm);
}

const char *kinc_language() {
	JNIEnv *env;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	jstring s = (jstring)(*env)->CallStaticObjectMethod(env, koreActivityClass,
	                                                    (*env)->GetStaticMethodID(env, koreActivityClass, "getLanguage", "()Ljava/lang/String;"));
	const char *str = (*env)->GetStringUTFChars(env, s, 0);
	(*activity->vm)->DetachCurrentThread(activity->vm);
	return str;
}

bool kinc_vulkan_internal_get_size(int *width, int *height);

int kinc_android_width() {
	int width, height;
	if (kinc_vulkan_internal_get_size(&width, &height)) {
		return width;
	}
	else {
		return ANativeWindow_getWidth(app->window);
	}
}

int kinc_android_height() {
	int width, height;
	if (kinc_vulkan_internal_get_size(&width, &height)) {
		return height;
	}
	else {
		return ANativeWindow_getHeight(app->window);
	}
}

const char *kinc_internal_save_path() {
	return kinc_android_get_activity()->internalDataPath;
}

const char *kinc_system_id() {
	return "Android";
}

static const char *videoFormats[] = {"ts", NULL};

const char **kinc_video_formats() {
	return videoFormats;
}

void kinc_set_keep_screen_on(bool on) {
	if (on) {
		ANativeActivity_setWindowFlags(activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
	}
	else {
		ANativeActivity_setWindowFlags(activity, 0, AWINDOW_FLAG_KEEP_SCREEN_ON);
	}
}

#include <kinc/input/acceleration.h>
#include <kinc/input/rotation.h>
#include <kinc/window.h>
#include <sys/time.h>
#include <time.h>

static __kernel_time_t start_sec = 0;

double kinc_frequency() {
	return 1000000.0;
}

kinc_ticks_t kinc_timestamp() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (kinc_ticks_t)(now.tv_sec - start_sec) * 1000000 + (kinc_ticks_t)(now.tv_usec);
}

double kinc_time() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return (double)(now.tv_sec - start_sec) + (now.tv_usec / 1000000.0);
}

void kinc_internal_resize(int window, int width, int height);

bool kinc_internal_handle_messages(void) {
	kinc_mutex_lock(&unicode_mutex);
	for (int i = 0; i < unicode_stack_index; ++i) {
		kinc_internal_keyboard_trigger_key_press(unicode_stack[i]);
	}
	unicode_stack_index = 0;
	kinc_mutex_unlock(&unicode_mutex);

	int ident;
	int events;
	struct android_poll_source *source;

	while ((ident = ALooper_pollAll(paused ? -1 : 0, NULL, &events, (void **)&source)) >= 0) {
		if (source != NULL) {
			source->process(app, source);
		}

		if (ident == LOOPER_ID_USER) {
			if (accelerometerSensor != NULL) {
				ASensorEvent event;
				while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {
					if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
						kinc_internal_on_acceleration(event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
					else if (event.type == ASENSOR_TYPE_GYROSCOPE) {
						kinc_internal_on_rotation(event.vector.x, event.vector.y, event.vector.z);
					}
				}
			}
		}

		if (app->destroyRequested != 0) {
			termDisplay();
			kinc_stop();
			return true;
		}
	}

	if (activityJustResized && app->window != NULL) {
		activityJustResized = false;
		int32_t width = kinc_android_width();
		int32_t height = kinc_android_height();
#ifdef KINC_VULKAN
		kinc_internal_resize(0, width, height);
#endif
		kinc_internal_call_resize_callback(0, width, height);
	}

	// Get screen rotation
	/*
	JNIEnv* env;
	(*activity->vm)->AttachCurrentThread(&env, NULL);
	jclass koreActivityClass = KoreAndroid::findClass(env, "tech.kode.kore.KoreActivity");
	jmethodID koreActivityGetRotation = (*env)->GetStaticMethodID(koreActivityClass, "getRotation", "()I");
	screenRotation = (*env)->CallStaticIntMethod(koreActivityClass, koreActivityGetRotation);
	(*activity->vm)->DetachCurrentThread();
	*/

	return true;
}

bool kinc_mouse_can_lock(void) {
	return false;
}

void kinc_mouse_show() {}

void kinc_mouse_hide() {}

void kinc_mouse_set_position(int window, int x, int y) {}

void kinc_internal_mouse_lock(int window) {}

void kinc_internal_mouse_unlock(void) {}

void kinc_mouse_get_position(int window, int *x, int *y) {
	x = 0;
	y = 0;
}

void kinc_mouse_set_cursor(int cursor_index) {}

bool kinc_gamepad_connected(int num) {
	return num == 0;
}

void kinc_gamepad_rumble(int gamepad, float left, float right) {}

void initAndroidFileReader();
void KoreAndroidVideoInit();

void android_main(struct android_app *application) {
	app_dummy();

	struct timeval now;
	gettimeofday(&now, NULL);
	start_sec = now.tv_sec;

	app = application;
	activity = application->activity;
	initAndroidFileReader();
	KoreAndroidVideoInit();
	KincAndroidKeyboardInit();
	application->onAppCmd = cmd;
	application->onInputEvent = input;
	activity->callbacks->onNativeWindowResized = resize;
	sensorManager = ASensorManager_getInstance();
	accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	gyroSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_GYROSCOPE);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager, application->looper, LOOPER_ID_USER, NULL, NULL);

	JNIEnv *env = NULL;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);

	jclass koreMoviePlayerClass = kinc_android_find_class(env, "tech.kinc.KincMoviePlayer");
	jmethodID updateAll = (*env)->GetStaticMethodID(env, koreMoviePlayerClass, "updateAll", "()V");

	while (!started) {
		kinc_internal_handle_messages();
		(*env)->CallStaticVoidMethod(env, koreMoviePlayerClass, updateAll);
	}
	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
	kickstart(0, NULL);

	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	jmethodID FinishHim = (*env)->GetStaticMethodID(env, koreActivityClass, "stop", "()V");
	(*env)->CallStaticVoidMethod(env, koreActivityClass, FinishHim);
	(*activity->vm)->DetachCurrentThread(activity->vm);
}

int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame) {
	kinc_mutex_init(&unicode_mutex);

	kinc_window_options_t default_win;
	if (win == NULL) {
		kinc_window_options_set_defaults(&default_win);
		win = &default_win;
	}
	win->width = width;
	win->height = height;

	kinc_framebuffer_options_t default_frame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&default_frame);
		frame = &default_frame;
	}

	kinc_g4_internal_init();
	kinc_g4_internal_init_window(0, frame->depth_bits, true);

	kinc_internal_gamepad_trigger_connect(0);

	return 0;
}

void kinc_internal_shutdown(void) {
	kinc_internal_gamepad_trigger_disconnect(0);
}

const char *kinc_gamepad_vendor(int gamepad) {
	return "Google";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "gamepad";
}

#include <kinc/io/filereader.h>

#define CLASS_NAME "android/app/NativeActivity"

void initAndroidFileReader(void) {
	if (activity == NULL) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Android activity is NULL");
		return;
	}

	JNIEnv *env;

	(*activity->vm)->AttachCurrentThread(activity->vm, &env, NULL);

	jclass android_app_NativeActivity = (*env)->FindClass(env, CLASS_NAME);
	jmethodID getExternalFilesDir = (*env)->GetMethodID(env, android_app_NativeActivity, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
	jobject file = (*env)->CallObjectMethod(env, activity->clazz, getExternalFilesDir, NULL);
	jclass java_io_File = (*env)->FindClass(env, "java/io/File");
	jmethodID getPath = (*env)->GetMethodID(env, java_io_File, "getPath", "()Ljava/lang/String;");
	jstring jPath = (*env)->CallObjectMethod(env, file, getPath);

	const char *path = (*env)->GetStringUTFChars(env, jPath, NULL);
	char *externalFilesDir = malloc(strlen(path) + 1);
	strcpy(externalFilesDir, path);
	kinc_internal_set_files_location(externalFilesDir);

	(*env)->ReleaseStringUTFChars(env, jPath, path);
	(*env)->DeleteLocalRef(env, jPath);
	(*activity->vm)->DetachCurrentThread(activity->vm);
}

static bool kinc_aasset_reader_close(kinc_file_reader_t *reader) {
	AAsset_close((struct AAsset *)reader->data);
	return true;
}

static size_t kinc_aasset_reader_read(kinc_file_reader_t *reader, void *data, size_t size) {
	return AAsset_read((struct AAsset *)reader->data, data, size);
}

static size_t kinc_aasset_reader_pos(kinc_file_reader_t *reader) {
	return (size_t)AAsset_seek((struct AAsset *)reader->data, 0, SEEK_CUR);
}

static bool kinc_aasset_reader_seek(kinc_file_reader_t *reader, size_t pos) {
	AAsset_seek((struct AAsset *)reader->data, pos, SEEK_SET);
	return true;
}

static bool kinc_aasset_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	if (type != KINC_FILE_TYPE_ASSET)
		return false;
	reader->data = AAssetManager_open(kinc_android_get_asset_manager(), filename, AASSET_MODE_RANDOM);
	if (reader->data == NULL)
		return false;
	reader->size = AAsset_getLength((struct AAsset *)reader->data);
	reader->close = kinc_aasset_reader_close;
	reader->read = kinc_aasset_reader_read;
	reader->pos = kinc_aasset_reader_pos;
	reader->seek = kinc_aasset_reader_seek;
	return true;
}

bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	memset(reader, 0, sizeof(*reader));
	return kinc_internal_file_reader_callback(reader, filename, type) ||
	       kinc_internal_file_reader_open(reader, filename, type) ||
	       kinc_aasset_reader_open(reader, filename, type);
}

int kinc_cpu_cores(void) {
	return kinc_hardware_threads();
}

int kinc_hardware_threads(void) {
	return sysconf(_SC_NPROCESSORS_ONLN);
}
