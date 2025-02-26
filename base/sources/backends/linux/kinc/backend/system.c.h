#include "iron_gpu.h"
#include <iron_input.h>
#include <iron_video.h>
#include <iron_system.h>
#include "gamepad.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "x11.h"
#include <X11/Xlib.h>
#include <ctype.h>
#include <dlfcn.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

#define Button6 6
#define Button7 7

struct kinc_x11_procs xlib = {0};
struct x11_context x11_ctx = {0};

static size_t clipboardStringSize = 1024;
static char *clipboardString = NULL;

char buffer[1024];

void kinc_internal_resize(int width, int height);
static void init_pen_device(XDeviceInfo *info, struct x11_pen_device *pen, bool eraser);

static void load_lib(void **lib, const char *name) {
	char libname[64];
	sprintf(libname, "lib%s.so", name);
	*lib = dlopen(libname, RTLD_LAZY);
	if (*lib != NULL) {
		return;
	}
	// Ubuntu and Fedora only ship libFoo.so.major by default, so look for those.
	for (int i = 0; i < 10; i++) {
		sprintf(libname, "lib%s.so.%i", name, i);
		*lib = dlopen(libname, RTLD_LAZY);
		if (*lib != NULL) {
			return;
		}
	}
}

int kinc_x11_error_handler(Display *display, XErrorEvent *error_event) {
	xlib.XGetErrorText(display, error_event->error_code, buffer, 1024);
	kinc_log(KINC_LOG_LEVEL_ERROR, "X Error: %s", buffer);
	kinc_debug_break();
	return 0;
}

bool kinc_x11_init() {
#undef LOAD_LIB
#undef LOAD_FUN
#define LOAD_LIB(name)                                                                                                                                         \
	{                                                                                                                                                          \
		load_lib(&x11_ctx.libs.name, #name);                                                                                                                   \
                                                                                                                                                               \
		if (x11_ctx.libs.name == NULL) {                                                                                                                       \
			kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to load lib%s.so", #name);                                                                                  \
			return false;                                                                                                                                      \
		}                                                                                                                                                      \
	}                                                                                                                                                          \
	// manually check for libX11, and return false if not present
	// only error for further libs

	load_lib(&x11_ctx.libs.X11, "X11");
	if (x11_ctx.libs.X11 == NULL) {
		return false;
	}

	LOAD_LIB(Xi);
	LOAD_LIB(Xcursor);
	LOAD_LIB(Xinerama);
	LOAD_LIB(Xrandr);
#define LOAD_FUN(lib, symbol)                                                                                                                                  \
	xlib.symbol = dlsym(x11_ctx.libs.lib, #symbol);                                                                                                            \
	if (xlib.symbol == NULL) {                                                                                                                                 \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Did not find symbol %s in library %s.", #symbol, #lib);                                                                \
	}
	LOAD_FUN(X11, XOpenDisplay)
	LOAD_FUN(X11, XCloseDisplay)
	LOAD_FUN(X11, XSetErrorHandler)
	LOAD_FUN(X11, XGetErrorText)
	LOAD_FUN(X11, XInternAtoms)
	LOAD_FUN(X11, XPending)
	LOAD_FUN(X11, XFlush)
	LOAD_FUN(X11, XNextEvent)
	LOAD_FUN(X11, XRefreshKeyboardMapping)
	LOAD_FUN(X11, XwcLookupString)
	LOAD_FUN(X11, XFilterEvent)
	LOAD_FUN(X11, XConvertSelection)
	LOAD_FUN(X11, XSetSelectionOwner)
	LOAD_FUN(X11, XLookupString)
	LOAD_FUN(X11, XkbKeycodeToKeysym)
	LOAD_FUN(X11, XSendEvent)
	LOAD_FUN(X11, XGetWindowProperty)
	LOAD_FUN(X11, XFree)
	LOAD_FUN(X11, XChangeProperty)
	LOAD_FUN(X11, XDefineCursor)
	LOAD_FUN(X11, XUndefineCursor)
	LOAD_FUN(X11, XCreateBitmapFromData)
	LOAD_FUN(X11, XCreatePixmapCursor)
	LOAD_FUN(X11, XFreePixmap)
	LOAD_FUN(Xcursor, XcursorLibraryLoadCursor)
	LOAD_FUN(X11, XWarpPointer)
	LOAD_FUN(X11, XQueryPointer)
	LOAD_FUN(X11, XCreateColormap)
	LOAD_FUN(X11, XCreateWindow)
	LOAD_FUN(X11, XMoveWindow)
	LOAD_FUN(X11, XResizeWindow)
	LOAD_FUN(X11, XDestroyWindow)
	LOAD_FUN(X11, XSetClassHint)
	LOAD_FUN(X11, XSetLocaleModifiers)
	LOAD_FUN(X11, XOpenIM)
	LOAD_FUN(X11, XCloseIM)
	LOAD_FUN(X11, XCreateIC)
	LOAD_FUN(X11, XDestroyIC)
	LOAD_FUN(X11, XSetICFocus)
	LOAD_FUN(X11, XMapWindow)
	LOAD_FUN(X11, XUnmapWindow)
	LOAD_FUN(X11, XSetWMProtocols)
	LOAD_FUN(X11, XPeekEvent)

	LOAD_FUN(Xi, XListInputDevices)
	LOAD_FUN(Xi, XFreeDeviceList)
	LOAD_FUN(Xi, XOpenDevice)
	LOAD_FUN(Xi, XCloseDevice)
	LOAD_FUN(Xi, XSelectExtensionEvent)

	LOAD_FUN(Xinerama, XineramaQueryExtension)
	LOAD_FUN(Xinerama, XineramaIsActive)
	LOAD_FUN(Xinerama, XineramaQueryScreens)
	LOAD_FUN(Xrandr, XRRGetScreenResourcesCurrent)
	LOAD_FUN(Xrandr, XRRGetOutputPrimary)
	LOAD_FUN(Xrandr, XRRGetOutputInfo)
	LOAD_FUN(Xrandr, XRRFreeOutputInfo)
	LOAD_FUN(Xrandr, XRRGetCrtcInfo)
	LOAD_FUN(Xrandr, XRRFreeCrtcInfo)
	LOAD_FUN(Xrandr, XRRFreeScreenResources)

#undef LOAD_FUN
#undef LOAD_LIB

	x11_ctx.display = xlib.XOpenDisplay(NULL);
	if (!x11_ctx.display) {
		return false;
	}

	xlib.XSetErrorHandler(kinc_x11_error_handler);

	// this should be kept in sync with the x11_atoms struct
	static char *atom_names[] = {
	    "XdndAware",
	    "XdndDrop",
	    "XdndEnter",
	    "text/uri-list",
	    "XdndStatus",
	    "XdndActionCopy",
	    "XdndSelection",
	    "CLIPBOARD",
	    "UTF8_STRING",
	    "XSEL_DATA",
	    "TARGETS",
	    "MULTIPLE",
	    "text/plain;charset=utf-8",
	    "WM_DELETE_WINDOW",
	    "_MOTIF_WM_HINTS",
	    "_NET_WM_NAME",
	    "_NET_WM_ICON_NAME",
	    "_NET_WM_STATE",
	    "_NET_WM_STATE_FULLSCREEN",
	    XI_MOUSE,
	    XI_TABLET,
	    XI_KEYBOARD,
	    XI_TOUCHSCREEN,
	    XI_TOUCHPAD,
	    XI_BUTTONBOX,
	    XI_BARCODE,
	    XI_TRACKBALL,
	    XI_QUADRATURE,
	    XI_ID_MODULE,
	    XI_ONE_KNOB,
	    XI_NINE_KNOB,
	    XI_KNOB_BOX,
	    XI_SPACEBALL,
	    XI_DATAGLOVE,
	    XI_EYETRACKER,
	    XI_CURSORKEYS,
	    XI_FOOTMOUSE,
	    XI_JOYSTICK,
	};

	assert((sizeof atom_names / sizeof atom_names[0]) == (sizeof(struct kinc_x11_atoms) / sizeof(Atom)));
	xlib.XInternAtoms(x11_ctx.display, atom_names, sizeof atom_names / sizeof atom_names[0], False, (Atom *)&x11_ctx.atoms);
	clipboardString = (char *)malloc(clipboardStringSize);

	x11_ctx.pen.id = -1;
	x11_ctx.eraser.id = -1;

	int count;
	XDeviceInfoPtr devices = (XDeviceInfoPtr)xlib.XListInputDevices(x11_ctx.display, &count);
	for (int i = 0; i < count; i++) {
		strncpy(buffer, devices[i].name, 1023);
		buffer[1023] = 0;
		for (int j = 0; buffer[j]; j++) {
			buffer[j] = tolower(buffer[j]);
		}

		if (strstr(buffer, "stylus") || strstr(buffer, "pen") || strstr(buffer, "wacom")) {
			init_pen_device(&devices[i], &x11_ctx.pen, false);
		}
		if (strstr(buffer, "eraser")) {
			init_pen_device(&devices[i], &x11_ctx.eraser, true);
		}
	}

	if (devices != NULL) {
		xlib.XFreeDeviceList(devices);
	}

	return true;
}

static void init_pen_device(XDeviceInfo *info, struct x11_pen_device *pen, bool eraser) {
	XDevice *device = xlib.XOpenDevice(x11_ctx.display, info->id);
	XAnyClassPtr c = info->inputclassinfo;
	for (int j = 0; j < device->num_classes; j++) {
		if (c->class == ValuatorClass) {
			XValuatorInfo *valuator_info = (XValuatorInfo *)c;
			if (valuator_info->num_axes > 2) {
				pen->maxPressure = valuator_info->axes[2].max_value;
			}
			pen->id = info->id;
			DeviceMotionNotify(device, pen->motionEvent, pen->motionClass);
			if (eraser) {
				pen->press = kinc_internal_eraser_trigger_press;
				pen->move = kinc_internal_eraser_trigger_move;
				pen->release = kinc_internal_eraser_trigger_release;
			}
			else {
				pen->press = kinc_internal_pen_trigger_press;
				pen->move = kinc_internal_pen_trigger_move;
				pen->release = kinc_internal_pen_trigger_release;
			}
			return;
		}
		c = (XAnyClassPtr)((uint8_t *)c + c->length);
	}
	xlib.XCloseDevice(x11_ctx.display, device);
}

static void check_pen_device(struct kinc_x11_window *window, XEvent *event, struct x11_pen_device *pen) {
	if (event->type == pen->motionEvent) {
		XDeviceMotionEvent *motion = (XDeviceMotionEvent *)(event);
		if (motion->deviceid == pen->id) {
			float p = (float)motion->axis_data[2] / (float)pen->maxPressure;
			if (p > 0 && x11_ctx.pen.current_pressure == 0) {
				pen->press(motion->x, motion->y, p);
			}
			else if (p == 0 && pen->current_pressure > 0) {
				pen->release(motion->x, motion->y, p);
			}
			else if (p > 0) {
				pen->move(motion->x, motion->y, p);
			}
			pen->current_pressure = p;
		}
	}
}

struct kinc_x11_window *window_from_window(Window window) {
	for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
		if (x11_ctx.windows[i].window == window) {
			return &x11_ctx.windows[i];
		}
	}
	return NULL;
}

void kinc_vulkan_get_instance_extensions(const char **names, int *index, int max) {
	assert(*index + 1 < max);
	names[(*index)++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
}

VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	return vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, x11_ctx.display,
	                                                     DefaultVisual(x11_ctx.display, DefaultScreen(x11_ctx.display))->visualid);
}
VkResult kinc_vulkan_create_surface(VkInstance instance, VkSurfaceKHR *surface) {
	VkXlibSurfaceCreateInfoKHR info = {0};
	info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.dpy = x11_ctx.display;
	info.window = x11_ctx.windows[0].window;
	return vkCreateXlibSurfaceKHR(instance, &info, NULL, surface);
}

static bool _handle_messages() {
	static bool controlDown = false;
	static int ignoreKeycode = 0;
	static bool preventNextKeyDownEvent = false;

	while (xlib.XPending(x11_ctx.display)) {
		XEvent event;
		xlib.XNextEvent(x11_ctx.display, &event);
		Window window = event.xclient.window;
		struct kinc_x11_window *k_window = window_from_window(window);
		if (k_window == NULL) {
			continue;
		}
		check_pen_device(k_window, &event, &x11_ctx.pen);
		check_pen_device(k_window, &event, &x11_ctx.eraser);
		switch (event.type) {
		case MappingNotify: {
			xlib.XRefreshKeyboardMapping(&event.xmapping);
		} break;
		case KeyPress: {

			XKeyEvent *key = (XKeyEvent *)&event;
			KeySym keysym;

			wchar_t wchar;

			bool wcConverted = xlib.XwcLookupString(k_window->xInputContext, key, &wchar, 1, &keysym, NULL);

			bool isIgnoredKeySym = keysym == XK_Escape || keysym == XK_BackSpace || keysym == XK_Delete;
			if (!controlDown && !xlib.XFilterEvent(&event, window) && !isIgnoredKeySym) {

				if (wcConverted) {
					kinc_internal_keyboard_trigger_key_press(wchar);
				}
			}

			if (preventNextKeyDownEvent) {
				// this keypress is a repeated keystroke and should not lead to a keydown-event
				preventNextKeyDownEvent = false;
				continue;
			}

#define KEY(xkey, korekey)                                                                                                                                     \
	case xkey:                                                                                                                                                 \
		kinc_internal_keyboard_trigger_key_down(korekey);                                                                                                      \
		break;

			KeySym ksKey = xlib.XkbKeycodeToKeysym(x11_ctx.display, event.xkey.keycode, 0, 0);

			if (ksKey == XK_Control_L || ksKey == XK_Control_R) {
				controlDown = true;
			}
			else if (controlDown && (ksKey == XK_v || ksKey == XK_V)) {
				xlib.XConvertSelection(x11_ctx.display, x11_ctx.atoms.CLIPBOARD, x11_ctx.atoms.UTF8_STRING, x11_ctx.atoms.XSEL_DATA, window, CurrentTime);
			}
			else if (controlDown && (ksKey == XK_c || ksKey == XK_C)) {
				xlib.XSetSelectionOwner(x11_ctx.display, x11_ctx.atoms.CLIPBOARD, window, CurrentTime);
				char *text = kinc_internal_copy_callback();
				if (text != NULL)
					kinc_copy_to_clipboard(text);
			}
			else if (controlDown && (ksKey == XK_x || ksKey == XK_X)) {
				xlib.XSetSelectionOwner(x11_ctx.display, x11_ctx.atoms.CLIPBOARD, window, CurrentTime);
				char *text = kinc_internal_cut_callback();
				if (text != NULL)
					kinc_copy_to_clipboard(text);
			}

			if (event.xkey.keycode == ignoreKeycode) {
				break;
			}
			else {
				ignoreKeycode = event.xkey.keycode;
			}

			if (ksKey < 97 || ksKey > 122) {
				ksKey = keysym;
			}

			switch (ksKey) {
				KEY(XK_Right, KINC_KEY_RIGHT)
				KEY(XK_Left, KINC_KEY_LEFT)
				KEY(XK_Up, KINC_KEY_UP)
				KEY(XK_Down, KINC_KEY_DOWN)
				KEY(XK_space, KINC_KEY_SPACE)
				KEY(XK_BackSpace, KINC_KEY_BACKSPACE)
				KEY(XK_Tab, KINC_KEY_TAB)
				KEY(XK_Return, KINC_KEY_RETURN)
				KEY(XK_Shift_L, KINC_KEY_SHIFT)
				KEY(XK_Shift_R, KINC_KEY_SHIFT)
				KEY(XK_Control_L, KINC_KEY_CONTROL)
				KEY(XK_Control_R, KINC_KEY_CONTROL)
				KEY(XK_Alt_L, KINC_KEY_ALT)
				KEY(XK_Alt_R, KINC_KEY_ALT)
				KEY(XK_Delete, KINC_KEY_DELETE)
				KEY(XK_comma, KINC_KEY_COMMA)
				KEY(XK_period, KINC_KEY_PERIOD)
				KEY(XK_bracketleft, KINC_KEY_OPEN_BRACKET)
				KEY(XK_bracketright, KINC_KEY_CLOSE_BRACKET)
				KEY(XK_braceleft, KINC_KEY_OPEN_CURLY_BRACKET)
				KEY(XK_braceright, KINC_KEY_CLOSE_CURLY_BRACKET)
				KEY(XK_parenleft, KINC_KEY_OPEN_PAREN)
				KEY(XK_parenright, KINC_KEY_CLOSE_PAREN)
				KEY(XK_backslash, KINC_KEY_BACK_SLASH)
				KEY(XK_apostrophe, KINC_KEY_QUOTE)
				KEY(XK_colon, KINC_KEY_COLON)
				KEY(XK_semicolon, KINC_KEY_SEMICOLON)
				KEY(XK_minus, KINC_KEY_HYPHEN_MINUS)
				KEY(XK_underscore, KINC_KEY_UNDERSCORE)
				KEY(XK_slash, KINC_KEY_SLASH)
				KEY(XK_bar, KINC_KEY_PIPE)
				KEY(XK_question, KINC_KEY_QUESTIONMARK)
				KEY(XK_less, KINC_KEY_LESS_THAN)
				KEY(XK_greater, KINC_KEY_GREATER_THAN)
				KEY(XK_asterisk, KINC_KEY_ASTERISK)
				KEY(XK_ampersand, KINC_KEY_AMPERSAND)
				KEY(XK_asciicircum, KINC_KEY_CIRCUMFLEX)
				KEY(XK_percent, KINC_KEY_PERCENT)
				KEY(XK_dollar, KINC_KEY_DOLLAR)
				KEY(XK_numbersign, KINC_KEY_HASH)
				KEY(XK_at, KINC_KEY_AT)
				KEY(XK_exclam, KINC_KEY_EXCLAMATION)
				KEY(XK_equal, KINC_KEY_EQUALS)
				KEY(XK_plus, KINC_KEY_ADD)
				KEY(XK_quoteleft, KINC_KEY_BACK_QUOTE)
				KEY(XK_quotedbl, KINC_KEY_DOUBLE_QUOTE)
				KEY(XK_asciitilde, KINC_KEY_TILDE)
				KEY(XK_Pause, KINC_KEY_PAUSE)
				KEY(XK_Scroll_Lock, KINC_KEY_SCROLL_LOCK)
				KEY(XK_Home, KINC_KEY_HOME)
				KEY(XK_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_Page_Down, KINC_KEY_PAGE_DOWN)
				KEY(XK_End, KINC_KEY_END)
				KEY(XK_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Enter, KINC_KEY_RETURN)
				KEY(XK_KP_Multiply, KINC_KEY_MULTIPLY)
				KEY(XK_KP_Add, KINC_KEY_ADD)
				KEY(XK_KP_Subtract, KINC_KEY_SUBTRACT)
				KEY(XK_KP_Decimal, KINC_KEY_DECIMAL)
				KEY(XK_KP_Divide, KINC_KEY_DIVIDE)
				KEY(XK_KP_0, KINC_KEY_NUMPAD_0)
				KEY(XK_KP_1, KINC_KEY_NUMPAD_1)
				KEY(XK_KP_2, KINC_KEY_NUMPAD_2)
				KEY(XK_KP_3, KINC_KEY_NUMPAD_3)
				KEY(XK_KP_4, KINC_KEY_NUMPAD_4)
				KEY(XK_KP_5, KINC_KEY_NUMPAD_5)
				KEY(XK_KP_6, KINC_KEY_NUMPAD_6)
				KEY(XK_KP_7, KINC_KEY_NUMPAD_7)
				KEY(XK_KP_8, KINC_KEY_NUMPAD_8)
				KEY(XK_KP_9, KINC_KEY_NUMPAD_9)
				KEY(XK_KP_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Delete, KINC_KEY_DELETE)
				KEY(XK_KP_End, KINC_KEY_END)
				KEY(XK_KP_Home, KINC_KEY_HOME)
				KEY(XK_KP_Left, KINC_KEY_LEFT)
				KEY(XK_KP_Up, KINC_KEY_UP)
				KEY(XK_KP_Right, KINC_KEY_RIGHT)
				KEY(XK_KP_Down, KINC_KEY_DOWN)
				KEY(XK_KP_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_KP_Page_Down, KINC_KEY_PAGE_DOWN)
				KEY(XK_Menu, KINC_KEY_CONTEXT_MENU)
				KEY(XK_a, KINC_KEY_A)
				KEY(XK_b, KINC_KEY_B)
				KEY(XK_c, KINC_KEY_C)
				KEY(XK_d, KINC_KEY_D)
				KEY(XK_e, KINC_KEY_E)
				KEY(XK_f, KINC_KEY_F)
				KEY(XK_g, KINC_KEY_G)
				KEY(XK_h, KINC_KEY_H)
				KEY(XK_i, KINC_KEY_I)
				KEY(XK_j, KINC_KEY_J)
				KEY(XK_k, KINC_KEY_K)
				KEY(XK_l, KINC_KEY_L)
				KEY(XK_m, KINC_KEY_M)
				KEY(XK_n, KINC_KEY_N)
				KEY(XK_o, KINC_KEY_O)
				KEY(XK_p, KINC_KEY_P)
				KEY(XK_q, KINC_KEY_Q)
				KEY(XK_r, KINC_KEY_R)
				KEY(XK_s, KINC_KEY_S)
				KEY(XK_t, KINC_KEY_T)
				KEY(XK_u, KINC_KEY_U)
				KEY(XK_v, KINC_KEY_V)
				KEY(XK_w, KINC_KEY_W)
				KEY(XK_x, KINC_KEY_X)
				KEY(XK_y, KINC_KEY_Y)
				KEY(XK_z, KINC_KEY_Z)
				KEY(XK_1, KINC_KEY_1)
				KEY(XK_2, KINC_KEY_2)
				KEY(XK_3, KINC_KEY_3)
				KEY(XK_4, KINC_KEY_4)
				KEY(XK_5, KINC_KEY_5)
				KEY(XK_6, KINC_KEY_6)
				KEY(XK_7, KINC_KEY_7)
				KEY(XK_8, KINC_KEY_8)
				KEY(XK_9, KINC_KEY_9)
				KEY(XK_0, KINC_KEY_0)
				KEY(XK_Escape, KINC_KEY_ESCAPE)
				KEY(XK_F1, KINC_KEY_F1)
				KEY(XK_F2, KINC_KEY_F2)
				KEY(XK_F3, KINC_KEY_F3)
				KEY(XK_F4, KINC_KEY_F4)
				KEY(XK_F5, KINC_KEY_F5)
				KEY(XK_F6, KINC_KEY_F6)
				KEY(XK_F7, KINC_KEY_F7)
				KEY(XK_F8, KINC_KEY_F8)
				KEY(XK_F9, KINC_KEY_F9)
				KEY(XK_F10, KINC_KEY_F10)
				KEY(XK_F11, KINC_KEY_F11)
				KEY(XK_F12, KINC_KEY_F12)
			}
			break;
#undef KEY
		}
		case KeyRelease: {
			XKeyEvent *key = (XKeyEvent *)&event;

			// peek next-event to determine if this a repeated-keystroke
			XEvent nev;
			if (xlib.XPending(x11_ctx.display)) {
				xlib.XPeekEvent(x11_ctx.display, &nev);

				if (nev.type == KeyPress && nev.xkey.time == event.xkey.time && nev.xkey.keycode == event.xkey.keycode) {
					// repeated keystroke! prevent this keyup-event and next keydown-event from being fired
					preventNextKeyDownEvent = true;
					continue;
				}
			}
			KeySym keysym;

			char c;
			xlib.XLookupString(key, &c, 1, &keysym, NULL);

#define KEY(xkey, korekey)                                                                                                                                     \
	case xkey:                                                                                                                                                 \
		kinc_internal_keyboard_trigger_key_up(korekey);                                                                                                        \
		break;

			KeySym ksKey = xlib.XkbKeycodeToKeysym(x11_ctx.display, event.xkey.keycode, 0, 0);

			if (ksKey == XK_Control_L || ksKey == XK_Control_R) {
				controlDown = false;
			}

			if (event.xkey.keycode == ignoreKeycode) {
				ignoreKeycode = 0;
			}

			if (ksKey < 97 || ksKey > 122) {
				ksKey = keysym;
			}

			switch (ksKey) {
				KEY(XK_Right, KINC_KEY_RIGHT)
				KEY(XK_Left, KINC_KEY_LEFT)
				KEY(XK_Up, KINC_KEY_UP)
				KEY(XK_Down, KINC_KEY_DOWN)
				KEY(XK_space, KINC_KEY_SPACE)
				KEY(XK_BackSpace, KINC_KEY_BACKSPACE)
				KEY(XK_Tab, KINC_KEY_TAB)
				KEY(XK_Return, KINC_KEY_RETURN)
				KEY(XK_Shift_L, KINC_KEY_SHIFT)
				KEY(XK_Shift_R, KINC_KEY_SHIFT)
				KEY(XK_Control_L, KINC_KEY_CONTROL)
				KEY(XK_Control_R, KINC_KEY_CONTROL)
				KEY(XK_Alt_L, KINC_KEY_ALT)
				KEY(XK_ISO_Prev_Group, KINC_KEY_ALT) //// XK_ISO_Prev_Group received instead of XK_Alt_L?
				KEY(XK_Alt_R, KINC_KEY_ALT)
				KEY(XK_Delete, KINC_KEY_DELETE)
				KEY(XK_comma, KINC_KEY_COMMA)
				KEY(XK_period, KINC_KEY_PERIOD)
				KEY(XK_bracketleft, KINC_KEY_OPEN_BRACKET)
				KEY(XK_bracketright, KINC_KEY_CLOSE_BRACKET)
				KEY(XK_braceleft, KINC_KEY_OPEN_CURLY_BRACKET)
				KEY(XK_braceright, KINC_KEY_CLOSE_CURLY_BRACKET)
				KEY(XK_parenleft, KINC_KEY_OPEN_PAREN)
				KEY(XK_parenright, KINC_KEY_CLOSE_PAREN)
				KEY(XK_backslash, KINC_KEY_BACK_SLASH)
				KEY(XK_apostrophe, KINC_KEY_QUOTE)
				KEY(XK_colon, KINC_KEY_COLON)
				KEY(XK_semicolon, KINC_KEY_SEMICOLON)
				KEY(XK_minus, KINC_KEY_HYPHEN_MINUS)
				KEY(XK_underscore, KINC_KEY_UNDERSCORE)
				KEY(XK_slash, KINC_KEY_SLASH)
				KEY(XK_bar, KINC_KEY_PIPE)
				KEY(XK_question, KINC_KEY_QUESTIONMARK)
				KEY(XK_less, KINC_KEY_LESS_THAN)
				KEY(XK_greater, KINC_KEY_GREATER_THAN)
				KEY(XK_asterisk, KINC_KEY_ASTERISK)
				KEY(XK_ampersand, KINC_KEY_AMPERSAND)
				KEY(XK_asciicircum, KINC_KEY_CIRCUMFLEX)
				KEY(XK_percent, KINC_KEY_PERCENT)
				KEY(XK_dollar, KINC_KEY_DOLLAR)
				KEY(XK_numbersign, KINC_KEY_HASH)
				KEY(XK_at, KINC_KEY_AT)
				KEY(XK_exclam, KINC_KEY_EXCLAMATION)
				KEY(XK_equal, KINC_KEY_EQUALS)
				KEY(XK_plus, KINC_KEY_ADD)
				KEY(XK_quoteleft, KINC_KEY_BACK_QUOTE)
				KEY(XK_quotedbl, KINC_KEY_DOUBLE_QUOTE)
				KEY(XK_asciitilde, KINC_KEY_TILDE)
				KEY(XK_Pause, KINC_KEY_PAUSE)
				KEY(XK_Scroll_Lock, KINC_KEY_SCROLL_LOCK)
				KEY(XK_Home, KINC_KEY_HOME)
				KEY(XK_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_Page_Down, KINC_KEY_PAGE_DOWN)
				KEY(XK_End, KINC_KEY_END)
				KEY(XK_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Enter, KINC_KEY_RETURN)
				KEY(XK_KP_Multiply, KINC_KEY_MULTIPLY)
				KEY(XK_KP_Add, KINC_KEY_ADD)
				KEY(XK_KP_Subtract, KINC_KEY_SUBTRACT)
				KEY(XK_KP_Decimal, KINC_KEY_DECIMAL)
				KEY(XK_KP_Divide, KINC_KEY_DIVIDE)
				KEY(XK_KP_0, KINC_KEY_NUMPAD_0)
				KEY(XK_KP_1, KINC_KEY_NUMPAD_1)
				KEY(XK_KP_2, KINC_KEY_NUMPAD_2)
				KEY(XK_KP_3, KINC_KEY_NUMPAD_3)
				KEY(XK_KP_4, KINC_KEY_NUMPAD_4)
				KEY(XK_KP_5, KINC_KEY_NUMPAD_5)
				KEY(XK_KP_6, KINC_KEY_NUMPAD_6)
				KEY(XK_KP_7, KINC_KEY_NUMPAD_7)
				KEY(XK_KP_8, KINC_KEY_NUMPAD_8)
				KEY(XK_KP_9, KINC_KEY_NUMPAD_9)
				KEY(XK_KP_Insert, KINC_KEY_INSERT)
				KEY(XK_KP_Delete, KINC_KEY_DELETE)
				KEY(XK_KP_End, KINC_KEY_END)
				KEY(XK_KP_Home, KINC_KEY_HOME)
				KEY(XK_KP_Left, KINC_KEY_LEFT)
				KEY(XK_KP_Up, KINC_KEY_UP)
				KEY(XK_KP_Right, KINC_KEY_RIGHT)
				KEY(XK_KP_Down, KINC_KEY_DOWN)
				KEY(XK_KP_Page_Up, KINC_KEY_PAGE_UP)
				KEY(XK_KP_Page_Down, KINC_KEY_PAGE_DOWN)
				KEY(XK_Menu, KINC_KEY_CONTEXT_MENU)
				KEY(XK_a, KINC_KEY_A)
				KEY(XK_b, KINC_KEY_B)
				KEY(XK_c, KINC_KEY_C)
				KEY(XK_d, KINC_KEY_D)
				KEY(XK_e, KINC_KEY_E)
				KEY(XK_f, KINC_KEY_F)
				KEY(XK_g, KINC_KEY_G)
				KEY(XK_h, KINC_KEY_H)
				KEY(XK_i, KINC_KEY_I)
				KEY(XK_j, KINC_KEY_J)
				KEY(XK_k, KINC_KEY_K)
				KEY(XK_l, KINC_KEY_L)
				KEY(XK_m, KINC_KEY_M)
				KEY(XK_n, KINC_KEY_N)
				KEY(XK_o, KINC_KEY_O)
				KEY(XK_p, KINC_KEY_P)
				KEY(XK_q, KINC_KEY_Q)
				KEY(XK_r, KINC_KEY_R)
				KEY(XK_s, KINC_KEY_S)
				KEY(XK_t, KINC_KEY_T)
				KEY(XK_u, KINC_KEY_U)
				KEY(XK_v, KINC_KEY_V)
				KEY(XK_w, KINC_KEY_W)
				KEY(XK_x, KINC_KEY_X)
				KEY(XK_y, KINC_KEY_Y)
				KEY(XK_z, KINC_KEY_Z)
				KEY(XK_1, KINC_KEY_1)
				KEY(XK_2, KINC_KEY_2)
				KEY(XK_3, KINC_KEY_3)
				KEY(XK_4, KINC_KEY_4)
				KEY(XK_5, KINC_KEY_5)
				KEY(XK_6, KINC_KEY_6)
				KEY(XK_7, KINC_KEY_7)
				KEY(XK_8, KINC_KEY_8)
				KEY(XK_9, KINC_KEY_9)
				KEY(XK_0, KINC_KEY_0)
				KEY(XK_Escape, KINC_KEY_ESCAPE)
				KEY(XK_F1, KINC_KEY_F1)
				KEY(XK_F2, KINC_KEY_F2)
				KEY(XK_F3, KINC_KEY_F3)
				KEY(XK_F4, KINC_KEY_F4)
				KEY(XK_F5, KINC_KEY_F5)
				KEY(XK_F6, KINC_KEY_F6)
				KEY(XK_F7, KINC_KEY_F7)
				KEY(XK_F8, KINC_KEY_F8)
				KEY(XK_F9, KINC_KEY_F9)
				KEY(XK_F10, KINC_KEY_F10)
				KEY(XK_F11, KINC_KEY_F11)
				KEY(XK_F12, KINC_KEY_F12)
			}
			break;
#undef KEY
		}
		case ButtonPress: {
			XButtonEvent *button = (XButtonEvent *)&event;

			switch (button->button) {
			case Button1:
				kinc_internal_mouse_trigger_press(0, button->x, button->y);
				break;
			case Button2:
				kinc_internal_mouse_trigger_press(2, button->x, button->y);
				break;
			case Button3:
				kinc_internal_mouse_trigger_press(1, button->x, button->y);
				break;
			// buttons 4-7 are for mouse wheel events because why not
			case Button4:
			case Button5:
			case Button6:
			case Button7:
				break;
			default:
				kinc_internal_mouse_trigger_press(button->button - Button1 - 4, button->x, button->y);
				break;
			}
			break;
		}
		case ButtonRelease: {
			XButtonEvent *button = (XButtonEvent *)&event;

			switch (button->button) {
			case Button1:
				kinc_internal_mouse_trigger_release(0, button->x, button->y);
				break;
			case Button2:
				kinc_internal_mouse_trigger_release(2, button->x, button->y);
				break;
			case Button3:
				kinc_internal_mouse_trigger_release(1, button->x, button->y);
				break;
			// Button4 and Button5 provide mouse wheel events because why not
			case Button4:
				kinc_internal_mouse_trigger_scroll(-1);
				break;
			case Button5:
				kinc_internal_mouse_trigger_scroll(1);
				break;
			// button 6 and 7 seem to be horizontal scrolling, which is not exposed in Kinc's api at the moment
			case Button6:
			case Button7:
				break;
			default:
				kinc_internal_mouse_trigger_release(button->button - Button1 - 4, button->x, button->y);
				break;
			}
			break;
		}
		case MotionNotify: {
			XMotionEvent *motion = (XMotionEvent *)&event;
			kinc_internal_mouse_trigger_move(motion->x, motion->y);
			break;
		}
		case ConfigureNotify: {
			if (event.xconfigure.width != k_window->width || event.xconfigure.height != k_window->height) {
				k_window->width = event.xconfigure.width;
				k_window->height = event.xconfigure.height;
				kinc_internal_resize(event.xconfigure.width, event.xconfigure.height);
				kinc_internal_call_resize_callback(event.xconfigure.width, event.xconfigure.height);
			}
			break;
		}
		case ClientMessage: {
			if (event.xclient.message_type == x11_ctx.atoms.XdndEnter) {
				Window source_window = event.xclient.data.l[0];
				XEvent m;
				memset(&m, 0, sizeof(m));
				m.type = ClientMessage;
				m.xclient.window = event.xclient.data.l[0];
				m.xclient.message_type = x11_ctx.atoms.XdndStatus;
				m.xclient.format = 32;
				m.xclient.data.l[0] = window;
				m.xclient.data.l[2] = 0;
				m.xclient.data.l[3] = 0;
				m.xclient.data.l[1] = 1;
				m.xclient.data.l[4] = x11_ctx.atoms.XdndActionCopy;
				xlib.XSendEvent(x11_ctx.display, source_window, false, NoEventMask, (XEvent *)&m);
				xlib.XFlush(x11_ctx.display);
			}
			else if (event.xclient.message_type == x11_ctx.atoms.XdndDrop) {
				xlib.XConvertSelection(x11_ctx.display, x11_ctx.atoms.XdndSelection, x11_ctx.atoms.XdndTextUriList, x11_ctx.atoms.XdndSelection, window,
				                       event.xclient.data.l[2]);
			}
			else if (event.xclient.data.l[0] == x11_ctx.atoms.WM_DELETE_WINDOW) {
				if (kinc_internal_call_close_callback()) {
					kinc_window_destroy();
					kinc_stop();
				}
			}
		}; break;
		case SelectionNotify: {
			if (event.xselection.selection == x11_ctx.atoms.CLIPBOARD) {
				char *result;
				unsigned long ressize, restail;
				int resbits;
				xlib.XGetWindowProperty(x11_ctx.display, window, x11_ctx.atoms.XSEL_DATA, 0, LONG_MAX / 4, False, AnyPropertyType, &x11_ctx.atoms.UTF8_STRING,
				                        &resbits, &ressize, &restail, (unsigned char **)&result);
				kinc_internal_paste_callback(result);
				xlib.XFree(result);
			}
			else if (event.xselection.property == x11_ctx.atoms.XdndSelection) {
				Atom type;
				int format;
				unsigned long numItems;
				unsigned long bytesAfter = 1;
				unsigned char *data = 0;
				xlib.XGetWindowProperty(x11_ctx.display, event.xselection.requestor, event.xselection.property, 0, LONG_MAX, False, event.xselection.target,
				                        &type, &format, &numItems, &bytesAfter, &data);
				size_t pos = 0;
				size_t len = 0;
				while (pos < numItems) {
					if (data[pos] == '\r') { // Found a file
						wchar_t filePath[len + 1];
						mbstowcs(filePath, buffer, len);
						filePath[len] = 0;
						kinc_internal_drop_files_callback(filePath + 7); // Strip file://
						pos += 2;                                        // Avoid \n
						len = 0;
					}
					buffer[len++] = data[pos++];
				}
				xlib.XFree(data);
			}
			break;
		}
		case SelectionRequest: {
			if (event.xselectionrequest.target == x11_ctx.atoms.TARGETS) {
				XEvent send;
				send.xselection.type = SelectionNotify;
				send.xselection.requestor = event.xselectionrequest.requestor;
				send.xselection.selection = event.xselectionrequest.selection;
				send.xselection.target = event.xselectionrequest.target;
				send.xselection.property = event.xselectionrequest.property;
				send.xselection.time = event.xselectionrequest.time;
				Atom available[] = {x11_ctx.atoms.TARGETS, x11_ctx.atoms.MULTIPLE, x11_ctx.atoms.TEXT_PLAIN, x11_ctx.atoms.UTF8_STRING};
				xlib.XChangeProperty(x11_ctx.display, send.xselection.requestor, send.xselection.property, XA_ATOM, 32, PropModeReplace,
				                     (unsigned char *)&available[0], 4);
				xlib.XSendEvent(x11_ctx.display, send.xselection.requestor, True, 0, &send);
			}
			if (event.xselectionrequest.target == x11_ctx.atoms.TEXT_PLAIN || event.xselectionrequest.target == x11_ctx.atoms.UTF8_STRING) {
				XEvent send;
				send.xselection.type = SelectionNotify;
				send.xselection.requestor = event.xselectionrequest.requestor;
				send.xselection.selection = event.xselectionrequest.selection;
				send.xselection.target = event.xselectionrequest.target;
				send.xselection.property = event.xselectionrequest.property;
				send.xselection.time = event.xselectionrequest.time;
				xlib.XChangeProperty(x11_ctx.display, send.xselection.requestor, send.xselection.property, send.xselection.target, 8, PropModeReplace,
				                     (const unsigned char *)clipboardString, strlen(clipboardString));
				xlib.XSendEvent(x11_ctx.display, send.xselection.requestor, True, 0, &send);
			}
			break;
		}
		case Expose:
			break;
		case FocusIn: {
			kinc_internal_foreground_callback();
			break;
		}
		case FocusOut: {
			controlDown = false;
			ignoreKeycode = 0;
			kinc_internal_background_callback();
			break;
		}
		case LeaveNotify:
			break;
		case EnterNotify:
			break;
		}
	}

	return true;
}

bool kinc_internal_handle_messages() {
	if (!_handle_messages()) {
		return false;
	}

	kinc_linux_updateHIDGamepads();

	return true;
}

const char *kinc_system_id() {
	return "Linux";
}

void kinc_set_keep_screen_on(bool on) {}

void kinc_keyboard_show() {}

void kinc_keyboard_hide() {}

bool kinc_keyboard_active() {
	return true;
}

void kinc_load_url(const char *url) {
#define MAX_COMMAND_BUFFER_SIZE 256
#define HTTP "http://"
#define HTTPS "https://"
	if (strncmp(url, HTTP, sizeof(HTTP) - 1) == 0 || strncmp(url, HTTPS, sizeof(HTTPS) - 1) == 0) {
		char openUrlCommand[MAX_COMMAND_BUFFER_SIZE];
		snprintf(openUrlCommand, MAX_COMMAND_BUFFER_SIZE, "xdg-open %s", url);
		int err = system(openUrlCommand);
		if (err != 0) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Error opening url %s", url);
		}
	}
#undef HTTPS
#undef HTTP
#undef MAX_COMMAND_BUFFER_SIZE
}

const char *kinc_language() {
	return "en";
}

static char save[2000];
static bool saveInitialized = false;

const char *kinc_internal_save_path() {
	// first check for an existing directory in $HOME
	// if one exists, use it, else create one in $XDG_DATA_HOME
	// See: https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
	if (!saveInitialized) {
		const char *homedir;

		if ((homedir = getenv("HOME")) == NULL) {
			homedir = getpwuid(getuid())->pw_dir;
		}

		strcpy(save, homedir);
		strcat(save, "/.");
		strcat(save, kinc_application_name());
		strcat(save, "/");
		struct stat st;
		if (stat(save, &st) == 0) {
			// use existing folder in $HOME
		}
		else {
			// use XDG folder
			const char *data_home;
			if ((data_home = getenv("XDG_DATA_HOME")) == NULL) {
				// $XDG_DATA_HOME is not defined, fall back to the default, $HOME/.local/share
				strcpy(save, homedir);
				strcat(save, "/.local/share/");
			}
			else {
				// use $XDG_DATA_HOME
				strcpy(save, data_home);
				if (data_home[strlen(data_home) - 1] != '/') {
					strcat(save, "/");
				}
			}
			strcat(save, kinc_application_name());
			strcat(save, "/");
			int res = mkdir(save, 0700);
			if (res != 0 && errno != EEXIST) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not create save directory '%s'. Error %d", save, errno);
			}
		}

		saveInitialized = true;
	}
	return save;
}

static const char *videoFormats[] = {"ogv", NULL};

const char **kinc_video_formats() {
	return videoFormats;
}

#include <sys/time.h>
#include <time.h>

double kinc_frequency(void) {
	return 1000000.0;
}

static struct timeval start;

kinc_ticks_t kinc_timestamp(void) {
	struct timeval now;
	gettimeofday(&now, NULL);
	now.tv_sec -= start.tv_sec;
	now.tv_usec -= start.tv_usec;
	return (kinc_ticks_t)now.tv_sec * 1000000 + (kinc_ticks_t)now.tv_usec;
}

void kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_linux_initHIDGamepads();

	gettimeofday(&start, NULL);
	kinc_x11_init();
	kinc_display_init();

	kinc_set_application_name(name);

	kinc_g5_internal_init();

	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}
	win->width = width;
	win->height = height;
	if (win->title == NULL) {
		win->title = name;
	}

	kinc_window_create(win, frame);
}

void kinc_internal_shutdown() {
	kinc_g5_internal_destroy();
	kinc_linux_closeHIDGamepads();
	free(clipboardString);
	xlib.XCloseDisplay(x11_ctx.display);
	kinc_internal_shutdown_callback();
}

#ifndef KINC_NO_MAIN
int main(int argc, char **argv) {
	return kickstart(argc, argv);
}
#endif

void kinc_copy_to_clipboard(const char *text) {
	size_t textLength = strlen(text);
	if (textLength >= clipboardStringSize) {
		free(clipboardString);
		clipboardStringSize = textLength + 1;
		clipboardString = (char *)malloc(clipboardStringSize);
	}
	strcpy(clipboardString, text);
}

static int parse_number_at_end_of_line(char *line) {
	char *end = &line[strlen(line) - 2];
	int num = 0;
	int multi = 1;
	while (*end >= '0' && *end <= '9') {
		num += (*end - '0') * multi;
		multi *= 10;
		--end;
	}
	return num;
}

int kinc_cpu_cores(void) {
	char line[1024];
	FILE *file = fopen("/proc/cpuinfo", "r");

	if (file != NULL) {
		int cores[1024];
		memset(cores, 0, sizeof(cores));

		int cpu_count = 0;
		int physical_id = -1;
		int per_cpu_cores = -1;
		int processor_count = 0;

		while (fgets(line, sizeof(line), file)) {
			if (strncmp(line, "processor", 9) == 0) {
				++processor_count;
				if (physical_id >= 0 && per_cpu_cores > 0) {
					if (physical_id + 1 > cpu_count) {
						cpu_count = physical_id + 1;
					}
					cores[physical_id] = per_cpu_cores;
					physical_id = -1;
					per_cpu_cores = -1;
				}
			}
			else if (strncmp(line, "physical id", 11) == 0) {
				physical_id = parse_number_at_end_of_line(line);
			}
			else if (strncmp(line, "cpu cores", 9) == 0) {
				per_cpu_cores = parse_number_at_end_of_line(line);
			}
		}
		fclose(file);

		if (physical_id >= 0 && per_cpu_cores > 0) {
			if (physical_id + 1 > cpu_count) {
				cpu_count = physical_id + 1;
			}
			cores[physical_id] = per_cpu_cores;
		}

		int proper_cpu_count = 0;
		for (int i = 0; i < cpu_count; ++i) {
			proper_cpu_count += cores[i];
		}

		if (proper_cpu_count > 0) {
			return proper_cpu_count;
		}
		else {
			return processor_count == 0 ? 1 : processor_count;
		}
	}
	else {
		return 1;
	};
}

int kinc_hardware_threads(void) {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

#include <xkbcommon/xkbcommon.h>

int xkb_to_kinc(xkb_keysym_t symbol) {
#define KEY(xkb, kinc)                                                                                                                                         \
	case xkb:                                                                                                                                                  \
		return kinc;
	switch (symbol) {
		KEY(XKB_KEY_Right, KINC_KEY_RIGHT)
		KEY(XKB_KEY_Left, KINC_KEY_LEFT)
		KEY(XKB_KEY_Up, KINC_KEY_UP)
		KEY(XKB_KEY_Down, KINC_KEY_DOWN)
		KEY(XKB_KEY_space, KINC_KEY_SPACE)
		KEY(XKB_KEY_BackSpace, KINC_KEY_BACKSPACE)
		KEY(XKB_KEY_Tab, KINC_KEY_TAB)
		KEY(XKB_KEY_Return, KINC_KEY_RETURN)
		KEY(XKB_KEY_Shift_L, KINC_KEY_SHIFT)
		KEY(XKB_KEY_Shift_R, KINC_KEY_SHIFT)
		KEY(XKB_KEY_Control_L, KINC_KEY_CONTROL)
		KEY(XKB_KEY_Control_R, KINC_KEY_CONTROL)
		KEY(XKB_KEY_Alt_L, KINC_KEY_ALT)
		KEY(XKB_KEY_Alt_R, KINC_KEY_ALT)
		KEY(XKB_KEY_Delete, KINC_KEY_DELETE)
		KEY(XKB_KEY_comma, KINC_KEY_COMMA)
		KEY(XKB_KEY_period, KINC_KEY_PERIOD)
		KEY(XKB_KEY_bracketleft, KINC_KEY_OPEN_BRACKET)
		KEY(XKB_KEY_bracketright, KINC_KEY_CLOSE_BRACKET)
		KEY(XKB_KEY_braceleft, KINC_KEY_OPEN_CURLY_BRACKET)
		KEY(XKB_KEY_braceright, KINC_KEY_CLOSE_CURLY_BRACKET)
		KEY(XKB_KEY_parenleft, KINC_KEY_OPEN_PAREN)
		KEY(XKB_KEY_parenright, KINC_KEY_CLOSE_PAREN)
		KEY(XKB_KEY_backslash, KINC_KEY_BACK_SLASH)
		KEY(XKB_KEY_apostrophe, KINC_KEY_QUOTE)
		KEY(XKB_KEY_colon, KINC_KEY_COLON)
		KEY(XKB_KEY_semicolon, KINC_KEY_SEMICOLON)
		KEY(XKB_KEY_minus, KINC_KEY_HYPHEN_MINUS)
		KEY(XKB_KEY_underscore, KINC_KEY_UNDERSCORE)
		KEY(XKB_KEY_slash, KINC_KEY_SLASH)
		KEY(XKB_KEY_bar, KINC_KEY_PIPE)
		KEY(XKB_KEY_question, KINC_KEY_QUESTIONMARK)
		KEY(XKB_KEY_less, KINC_KEY_LESS_THAN)
		KEY(XKB_KEY_greater, KINC_KEY_GREATER_THAN)
		KEY(XKB_KEY_asterisk, KINC_KEY_ASTERISK)
		KEY(XKB_KEY_ampersand, KINC_KEY_AMPERSAND)
		KEY(XKB_KEY_asciicircum, KINC_KEY_CIRCUMFLEX)
		KEY(XKB_KEY_percent, KINC_KEY_PERCENT)
		KEY(XKB_KEY_dollar, KINC_KEY_DOLLAR)
		KEY(XKB_KEY_numbersign, KINC_KEY_HASH)
		KEY(XKB_KEY_at, KINC_KEY_AT)
		KEY(XKB_KEY_exclam, KINC_KEY_EXCLAMATION)
		KEY(XKB_KEY_equal, KINC_KEY_EQUALS)
		KEY(XKB_KEY_plus, KINC_KEY_ADD)
		KEY(XKB_KEY_quoteleft, KINC_KEY_BACK_QUOTE)
		KEY(XKB_KEY_quotedbl, KINC_KEY_DOUBLE_QUOTE)
		KEY(XKB_KEY_asciitilde, KINC_KEY_TILDE)
		KEY(XKB_KEY_Pause, KINC_KEY_PAUSE)
		KEY(XKB_KEY_Scroll_Lock, KINC_KEY_SCROLL_LOCK)
		KEY(XKB_KEY_Home, KINC_KEY_HOME)
		KEY(XKB_KEY_Page_Up, KINC_KEY_PAGE_UP)
		KEY(XKB_KEY_Page_Down, KINC_KEY_PAGE_DOWN)
		KEY(XKB_KEY_End, KINC_KEY_END)
		KEY(XKB_KEY_Insert, KINC_KEY_INSERT)
		KEY(XKB_KEY_KP_Enter, KINC_KEY_RETURN)
		KEY(XKB_KEY_KP_Multiply, KINC_KEY_MULTIPLY)
		KEY(XKB_KEY_KP_Add, KINC_KEY_ADD)
		KEY(XKB_KEY_KP_Subtract, KINC_KEY_SUBTRACT)
		KEY(XKB_KEY_KP_Decimal, KINC_KEY_DECIMAL)
		KEY(XKB_KEY_KP_Divide, KINC_KEY_DIVIDE)
		KEY(XKB_KEY_KP_0, KINC_KEY_NUMPAD_0)
		KEY(XKB_KEY_KP_1, KINC_KEY_NUMPAD_1)
		KEY(XKB_KEY_KP_2, KINC_KEY_NUMPAD_2)
		KEY(XKB_KEY_KP_3, KINC_KEY_NUMPAD_3)
		KEY(XKB_KEY_KP_4, KINC_KEY_NUMPAD_4)
		KEY(XKB_KEY_KP_5, KINC_KEY_NUMPAD_5)
		KEY(XKB_KEY_KP_6, KINC_KEY_NUMPAD_6)
		KEY(XKB_KEY_KP_7, KINC_KEY_NUMPAD_7)
		KEY(XKB_KEY_KP_8, KINC_KEY_NUMPAD_8)
		KEY(XKB_KEY_KP_9, KINC_KEY_NUMPAD_9)
		KEY(XKB_KEY_KP_Insert, KINC_KEY_INSERT)
		KEY(XKB_KEY_KP_Delete, KINC_KEY_DELETE)
		KEY(XKB_KEY_KP_End, KINC_KEY_END)
		KEY(XKB_KEY_KP_Home, KINC_KEY_HOME)
		KEY(XKB_KEY_KP_Left, KINC_KEY_LEFT)
		KEY(XKB_KEY_KP_Up, KINC_KEY_UP)
		KEY(XKB_KEY_KP_Right, KINC_KEY_RIGHT)
		KEY(XKB_KEY_KP_Down, KINC_KEY_DOWN)
		KEY(XKB_KEY_KP_Page_Up, KINC_KEY_PAGE_UP)
		KEY(XKB_KEY_KP_Page_Down, KINC_KEY_PAGE_DOWN)
		KEY(XKB_KEY_Menu, KINC_KEY_CONTEXT_MENU)
		KEY(XKB_KEY_a, KINC_KEY_A)
		KEY(XKB_KEY_b, KINC_KEY_B)
		KEY(XKB_KEY_c, KINC_KEY_C)
		KEY(XKB_KEY_d, KINC_KEY_D)
		KEY(XKB_KEY_e, KINC_KEY_E)
		KEY(XKB_KEY_f, KINC_KEY_F)
		KEY(XKB_KEY_g, KINC_KEY_G)
		KEY(XKB_KEY_h, KINC_KEY_H)
		KEY(XKB_KEY_i, KINC_KEY_I)
		KEY(XKB_KEY_j, KINC_KEY_J)
		KEY(XKB_KEY_k, KINC_KEY_K)
		KEY(XKB_KEY_l, KINC_KEY_L)
		KEY(XKB_KEY_m, KINC_KEY_M)
		KEY(XKB_KEY_n, KINC_KEY_N)
		KEY(XKB_KEY_o, KINC_KEY_O)
		KEY(XKB_KEY_p, KINC_KEY_P)
		KEY(XKB_KEY_q, KINC_KEY_Q)
		KEY(XKB_KEY_r, KINC_KEY_R)
		KEY(XKB_KEY_s, KINC_KEY_S)
		KEY(XKB_KEY_t, KINC_KEY_T)
		KEY(XKB_KEY_u, KINC_KEY_U)
		KEY(XKB_KEY_v, KINC_KEY_V)
		KEY(XKB_KEY_w, KINC_KEY_W)
		KEY(XKB_KEY_x, KINC_KEY_X)
		KEY(XKB_KEY_y, KINC_KEY_Y)
		KEY(XKB_KEY_z, KINC_KEY_Z)
		KEY(XKB_KEY_1, KINC_KEY_1)
		KEY(XKB_KEY_2, KINC_KEY_2)
		KEY(XKB_KEY_3, KINC_KEY_3)
		KEY(XKB_KEY_4, KINC_KEY_4)
		KEY(XKB_KEY_5, KINC_KEY_5)
		KEY(XKB_KEY_6, KINC_KEY_6)
		KEY(XKB_KEY_7, KINC_KEY_7)
		KEY(XKB_KEY_8, KINC_KEY_8)
		KEY(XKB_KEY_9, KINC_KEY_9)
		KEY(XKB_KEY_0, KINC_KEY_0)
		KEY(XKB_KEY_Escape, KINC_KEY_ESCAPE)
		KEY(XKB_KEY_F1, KINC_KEY_F1)
		KEY(XKB_KEY_F2, KINC_KEY_F2)
		KEY(XKB_KEY_F3, KINC_KEY_F3)
		KEY(XKB_KEY_F4, KINC_KEY_F4)
		KEY(XKB_KEY_F5, KINC_KEY_F5)
		KEY(XKB_KEY_F6, KINC_KEY_F6)
		KEY(XKB_KEY_F7, KINC_KEY_F7)
		KEY(XKB_KEY_F8, KINC_KEY_F8)
		KEY(XKB_KEY_F9, KINC_KEY_F9)
		KEY(XKB_KEY_F10, KINC_KEY_F10)
		KEY(XKB_KEY_F11, KINC_KEY_F11)
		KEY(XKB_KEY_F12, KINC_KEY_F12)
	default:
		return KINC_KEY_UNKNOWN;
	}
#undef KEY
}
