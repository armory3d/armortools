#include "linux_system.h"
#include <iron_gpu.h>
#include <iron_video.h>
#include <iron_system.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>

#define Button6 6
#define Button7 7

struct iron_x11_procs xlib = {0};
struct x11_context x11_ctx = {0};

static size_t clipboardStringSize = 1024;
static char *clipboardString = NULL;
static char buffer[1024];
static char save[2000];
static bool saveInitialized = false;
static const char *videoFormats[] = {"ogv", NULL};
static struct timeval start;
static bool mouse_hidden = false;

static void init_pen_device(XDeviceInfo *info, struct x11_pen_device *pen, bool eraser);
bool iron_x11_init();

void iron_display_init() {
	static bool display_initialized = false;
	if (display_initialized) {
		return;
	}

	iron_x11_init();

	Window root_window = RootWindow(x11_ctx.display, DefaultScreen(x11_ctx.display));
	XRRScreenResources *screen_resources = xlib.XRRGetScreenResourcesCurrent(x11_ctx.display, root_window);
	RROutput primary_output = xlib.XRRGetOutputPrimary(x11_ctx.display, root_window);

	for (int i = 0; i < screen_resources->noutput; i++) {
		if (i >= MAXIMUM_DISPLAYS) {
			iron_error("Too many screens (maximum %i)", MAXIMUM_DISPLAYS);
			break;
		}

		XRROutputInfo *output_info = xlib.XRRGetOutputInfo(x11_ctx.display, screen_resources, screen_resources->outputs[i]);
		if (output_info->connection != RR_Connected || output_info->crtc == None) {
			xlib.XRRFreeOutputInfo(output_info);
			continue;
		}

		XRRCrtcInfo *crtc_info = xlib.XRRGetCrtcInfo(x11_ctx.display, screen_resources, output_info->crtc);

		struct iron_x11_display *display = &x11_ctx.displays[x11_ctx.num_displays++];
		display->index = i;
		display->x = crtc_info->x;
		display->y = crtc_info->y;
		display->width = crtc_info->width;
		display->height = crtc_info->height;
		display->primary = screen_resources->outputs[i] == primary_output;
		display->crtc = output_info->crtc;
		display->output = screen_resources->outputs[i];

		xlib.XRRFreeOutputInfo(output_info);
		xlib.XRRFreeCrtcInfo(crtc_info);
	}

	xlib.XRRFreeScreenResources(screen_resources);
	display_initialized = true;
}

iron_display_mode_t iron_display_current_mode(int display_index) {
	struct iron_x11_display *display = &x11_ctx.displays[display_index];
	iron_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = display->width;
	mode.height = display->height;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	mode.pixels_per_inch = 96;

	Window root_window = DefaultRootWindow(x11_ctx.display);
	XRRScreenResources *screen_resources = xlib.XRRGetScreenResourcesCurrent(x11_ctx.display, root_window);

	XRROutputInfo *output_info = xlib.XRRGetOutputInfo(x11_ctx.display, screen_resources, screen_resources->outputs[display->index]);
	if (output_info->connection != RR_Connected || output_info->crtc == None) {
		xlib.XRRFreeOutputInfo(output_info);
		xlib.XRRFreeScreenResources(screen_resources);
		return mode;
	}

	XRRCrtcInfo *crtc_info = xlib.XRRGetCrtcInfo(x11_ctx.display, screen_resources, output_info->crtc);
	for (int j = 0; j < output_info->nmode; j++) {
		RRMode rr_mode = crtc_info->mode;
		XRRModeInfo *mode_info = NULL;
		for (int k = 0; k < screen_resources->nmode; k++) {
			if (screen_resources->modes[k].id == rr_mode) {
				mode_info = &screen_resources->modes[k];
				break;
			}
		}

		if (mode_info == NULL) {
			continue;
		}
		mode.x = display->x;
		mode.y = display->y;
		mode.width = mode_info->width;
		mode.height = mode_info->height;
		mode.pixels_per_inch = 96;
		mode.bits_per_pixel = 32;
		if (mode_info->hTotal && mode_info->vTotal) {
			mode.frequency = (mode_info->dotClock / (mode_info->hTotal * mode_info->vTotal));
		}
	}
	xlib.XRRFreeOutputInfo(output_info);
	xlib.XRRFreeCrtcInfo(crtc_info);
	xlib.XRRFreeScreenResources(screen_resources);
	return mode;
}

int iron_primary_display(void) {
	for (int i = 0; i < x11_ctx.num_displays; i++) {
		if (x11_ctx.displays[i].primary) {
			return i;
		}
	}
	return 0;
}

int iron_count_displays(void) {
	return x11_ctx.num_displays;
}

int iron_window_x() {
	return 0;
}

int iron_window_y() {
	return 0;
}

int iron_window_width() {
	return x11_ctx.windows[0].width;
}

int iron_window_height() {
	return x11_ctx.windows[0].height;
}

void iron_window_resize(int width, int height) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XResizeWindow(x11_ctx.display, window->window, width, height);
}

void iron_window_move(int x, int y) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XMoveWindow(x11_ctx.display, window->window, x, y);
}

void iron_window_change_mode(iron_window_mode_t mode) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	if (mode == window->mode) {
		return;
	}
	window->mode = mode;
	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = window->window;
	xev.xclient.message_type = x11_ctx.atoms.NET_WM_STATE;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = mode == IRON_WINDOW_MODE_FULLSCREEN ? 1 : 0;
	xev.xclient.data.l[1] = x11_ctx.atoms.NET_WM_STATE_FULLSCREEN;
	xev.xclient.data.l[2] = 0;
	xlib.XMapWindow(x11_ctx.display, window->window);
	xlib.XSendEvent(x11_ctx.display, DefaultRootWindow(x11_ctx.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	xlib.XFlush(x11_ctx.display);
}

int iron_window_display() {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	return window->display_index;
}

void iron_window_destroy() {
	xlib.XFlush(x11_ctx.display);
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XDestroyIC(window->xInputContext);
	xlib.XCloseIM(window->xInputMethod);
	xlib.XDestroyWindow(x11_ctx.display, window->window);
	xlib.XFlush(x11_ctx.display);
	*window = (struct iron_x11_window){0};
}

void iron_window_show() {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XMapWindow(x11_ctx.display, window->window);
}

void iron_window_hide() {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XUnmapWindow(x11_ctx.display, window->window);
}

void iron_window_set_title(const char *title) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XChangeProperty(x11_ctx.display, window->window, x11_ctx.atoms.NET_WM_NAME, x11_ctx.atoms.UTF8_STRING, 8, PropModeReplace, (unsigned char *)title, strlen(title));
	xlib.XChangeProperty(x11_ctx.display, window->window, x11_ctx.atoms.NET_WM_ICON_NAME, x11_ctx.atoms.UTF8_STRING, 8, PropModeReplace, (unsigned char *)title, strlen(title));
	xlib.XFlush(x11_ctx.display);
}

void iron_window_create(iron_window_options_t *win) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	window->width = win->width;
	window->height = win->height;

	Visual *visual = NULL;
	XSetWindowAttributes set_window_attribs = {0};

	XColor color;
	Colormap colormap = DefaultColormap(x11_ctx.display, DefaultScreen(x11_ctx.display));
	color.red = (32 * 65535) / 255;
	color.green = (32 * 65535) / 255;
	color.blue = (32 * 65535) / 255;
	xlib.XAllocColor(x11_ctx.display, colormap, &color);
	set_window_attribs.background_pixel = color.pixel;

	set_window_attribs.border_pixel = 0;
	set_window_attribs.event_mask =
		KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;
	int screen = DefaultScreen(x11_ctx.display);
	visual = DefaultVisual(x11_ctx.display, screen);
	int depth = DefaultDepth(x11_ctx.display, screen);
	set_window_attribs.colormap = xlib.XCreateColormap(x11_ctx.display, RootWindow(x11_ctx.display, screen), visual, AllocNone);
	window->window = xlib.XCreateWindow(x11_ctx.display, RootWindow(x11_ctx.display, DefaultScreen(x11_ctx.display)), 0, 0, win->width, win->height, 0, depth,
										InputOutput, visual, CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &set_window_attribs);

	static char nameClass[256];
	static const char *nameClassAddendum = "_IronApplication";
	strncpy(nameClass, iron_application_name(), sizeof(nameClass) - strlen(nameClassAddendum) - 1);
	strcat(nameClass, nameClassAddendum);
	char resNameBuffer[256];
	strncpy(resNameBuffer, iron_application_name(), 256);
	XClassHint classHint = {.res_name = resNameBuffer, .res_class = nameClass};
	xlib.XSetClassHint(x11_ctx.display, window->window, &classHint);

	xlib.XSetLocaleModifiers("@im=none");
	window->xInputMethod = xlib.XOpenIM(x11_ctx.display, NULL, NULL, NULL);
	window->xInputContext = xlib.XCreateIC(window->xInputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window->window, NULL);
	xlib.XSetICFocus(window->xInputContext);

	window->mode = IRON_WINDOW_MODE_WINDOW;
	iron_window_change_mode(win->mode);

	xlib.XMapWindow(x11_ctx.display, window->window);

	Atom XdndVersion = 5;
	xlib.XChangeProperty(x11_ctx.display, window->window, x11_ctx.atoms.XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char *)&XdndVersion, 1);
	xlib.XSetWMProtocols(x11_ctx.display, window->window, &x11_ctx.atoms.WM_DELETE_WINDOW, 1);

	iron_window_set_title(win->title);

	if (x11_ctx.pen.id != -1) {
		xlib.XSelectExtensionEvent(x11_ctx.display, window->window, &x11_ctx.pen.motionClass, 1);
	}
	if (x11_ctx.eraser.id != -1) {
		xlib.XSelectExtensionEvent(x11_ctx.display, window->window, &x11_ctx.eraser.motionClass, 1);
	}

	gpu_init(win->depth_bits, win->vsync);
}

static struct {
	void (*resize_callback)(int width, int height, void *data);
	void *resize_data;
	void *ppi_data;
	bool (*close_callback)(void *data);
	void *close_data;
} iron_internal_window_callbacks[1];

void iron_window_set_resize_callback(void (*callback)(int width, int height, void *data), void *data) {
	iron_internal_window_callbacks[0].resize_callback = callback;
	iron_internal_window_callbacks[0].resize_data = data;
}

void iron_internal_call_resize_callback(int width, int height) {
	if (iron_internal_window_callbacks[0].resize_callback != NULL) {
		iron_internal_window_callbacks[0].resize_callback(width, height, iron_internal_window_callbacks[0].resize_data);
	}
}

void iron_window_set_close_callback(bool (*callback)(void *data), void *data) {
	iron_internal_window_callbacks[0].close_callback = callback;
	iron_internal_window_callbacks[0].close_data = data;
}

bool iron_internal_call_close_callback() {
	if (iron_internal_window_callbacks[0].close_callback != NULL) {
		return iron_internal_window_callbacks[0].close_callback(iron_internal_window_callbacks[0].close_data);
	}
	else {
		return true;
	}
}

iron_window_mode_t iron_window_get_mode() {
	return x11_ctx.windows[0].mode;
}

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
	iron_error("Failed to load lib%s.so", name);
}

int iron_x11_error_handler(Display *display, XErrorEvent *error_event) {
	xlib.XGetErrorText(display, error_event->error_code, buffer, 1024);
	iron_error("X Error: %s", buffer);
	return 0;
}

bool iron_x11_init() {
	load_lib(&x11_ctx.libs.X11, "X11");
	load_lib(&x11_ctx.libs.Xi, "Xi");
	load_lib(&x11_ctx.libs.Xcursor, "Xcursor");
	load_lib(&x11_ctx.libs.Xrandr, "Xrandr");

	xlib.XOpenDisplay = dlsym(x11_ctx.libs.X11, "XOpenDisplay");
	xlib.XCloseDisplay = dlsym(x11_ctx.libs.X11, "XCloseDisplay");
	xlib.XInitThreads = dlsym(x11_ctx.libs.X11, "XInitThreads");
	xlib.XSetErrorHandler = dlsym(x11_ctx.libs.X11, "XSetErrorHandler");
	xlib.XGetErrorText = dlsym(x11_ctx.libs.X11, "XGetErrorText");
	xlib.XInternAtoms = dlsym(x11_ctx.libs.X11, "XInternAtoms");
	xlib.XPending = dlsym(x11_ctx.libs.X11, "XPending");
	xlib.XFlush = dlsym(x11_ctx.libs.X11, "XFlush");
	xlib.XNextEvent = dlsym(x11_ctx.libs.X11, "XNextEvent");
	xlib.XRefreshKeyboardMapping = dlsym(x11_ctx.libs.X11, "XRefreshKeyboardMapping");
	xlib.XwcLookupString = dlsym(x11_ctx.libs.X11, "XwcLookupString");
	xlib.XFilterEvent = dlsym(x11_ctx.libs.X11, "XFilterEvent");
	xlib.XConvertSelection = dlsym(x11_ctx.libs.X11, "XConvertSelection");
	xlib.XSetSelectionOwner = dlsym(x11_ctx.libs.X11, "XSetSelectionOwner");
	xlib.XLookupString = dlsym(x11_ctx.libs.X11, "XLookupString");
	xlib.XkbKeycodeToKeysym = dlsym(x11_ctx.libs.X11, "XkbKeycodeToKeysym");
	xlib.XSendEvent = dlsym(x11_ctx.libs.X11, "XSendEvent");
	xlib.XGetWindowProperty = dlsym(x11_ctx.libs.X11, "XGetWindowProperty");
	xlib.XFree = dlsym(x11_ctx.libs.X11, "XFree");
	xlib.XChangeProperty = dlsym(x11_ctx.libs.X11, "XChangeProperty");
	xlib.XDefineCursor = dlsym(x11_ctx.libs.X11, "XDefineCursor");
	xlib.XUndefineCursor = dlsym(x11_ctx.libs.X11, "XUndefineCursor");
	xlib.XCreateBitmapFromData = dlsym(x11_ctx.libs.X11, "XCreateBitmapFromData");
	xlib.XCreatePixmapCursor = dlsym(x11_ctx.libs.X11, "XCreatePixmapCursor");
	xlib.XFreePixmap = dlsym(x11_ctx.libs.X11, "XFreePixmap");
	xlib.XWarpPointer = dlsym(x11_ctx.libs.X11, "XWarpPointer");
	xlib.XQueryPointer = dlsym(x11_ctx.libs.X11, "XQueryPointer");
	xlib.XCreateColormap = dlsym(x11_ctx.libs.X11, "XCreateColormap");
	xlib.XCreateWindow = dlsym(x11_ctx.libs.X11, "XCreateWindow");
	xlib.XMoveWindow = dlsym(x11_ctx.libs.X11, "XMoveWindow");
	xlib.XResizeWindow = dlsym(x11_ctx.libs.X11, "XResizeWindow");
	xlib.XDestroyWindow = dlsym(x11_ctx.libs.X11, "XDestroyWindow");
	xlib.XSetClassHint = dlsym(x11_ctx.libs.X11, "XSetClassHint");
	xlib.XSetLocaleModifiers = dlsym(x11_ctx.libs.X11, "XSetLocaleModifiers");
	xlib.XOpenIM = dlsym(x11_ctx.libs.X11, "XOpenIM");
	xlib.XCloseIM = dlsym(x11_ctx.libs.X11, "XCloseIM");
	xlib.XCreateIC = dlsym(x11_ctx.libs.X11, "XCreateIC");
	xlib.XDestroyIC = dlsym(x11_ctx.libs.X11, "XDestroyIC");
	xlib.XSetICFocus = dlsym(x11_ctx.libs.X11, "XSetICFocus");
	xlib.XMapWindow = dlsym(x11_ctx.libs.X11, "XMapWindow");
	xlib.XUnmapWindow = dlsym(x11_ctx.libs.X11, "XUnmapWindow");
	xlib.XSetWMProtocols = dlsym(x11_ctx.libs.X11, "XSetWMProtocols");
	xlib.XPeekEvent = dlsym(x11_ctx.libs.X11, "XPeekEvent");
	xlib.XAllocColor = dlsym(x11_ctx.libs.X11, "XAllocColor");
	xlib.XListInputDevices = dlsym(x11_ctx.libs.Xi, "XListInputDevices");
	xlib.XFreeDeviceList = dlsym(x11_ctx.libs.Xi, "XFreeDeviceList");
	xlib.XOpenDevice = dlsym(x11_ctx.libs.Xi, "XOpenDevice");
	xlib.XCloseDevice = dlsym(x11_ctx.libs.Xi, "XCloseDevice");
	xlib.XSelectExtensionEvent = dlsym(x11_ctx.libs.Xi, "XSelectExtensionEvent");
	xlib.XcursorLibraryLoadCursor = dlsym(x11_ctx.libs.Xcursor, "XcursorLibraryLoadCursor");
	xlib.XRRGetScreenResourcesCurrent = dlsym(x11_ctx.libs.Xrandr, "XRRGetScreenResourcesCurrent");
	xlib.XRRGetOutputPrimary = dlsym(x11_ctx.libs.Xrandr, "XRRGetOutputPrimary");
	xlib.XRRGetOutputInfo = dlsym(x11_ctx.libs.Xrandr, "XRRGetOutputInfo");
	xlib.XRRFreeOutputInfo = dlsym(x11_ctx.libs.Xrandr, "XRRFreeOutputInfo");
	xlib.XRRGetCrtcInfo = dlsym(x11_ctx.libs.Xrandr, "XRRGetCrtcInfo");
	xlib.XRRFreeCrtcInfo = dlsym(x11_ctx.libs.Xrandr, "XRRFreeCrtcInfo");
	xlib.XRRFreeScreenResources = dlsym(x11_ctx.libs.Xrandr, "XRRFreeScreenResources");

	xlib.XInitThreads(); // Fixes random ubuntu22 crash

	x11_ctx.display = xlib.XOpenDisplay(NULL);
	if (!x11_ctx.display) {
		return false;
	}

	xlib.XSetErrorHandler(iron_x11_error_handler);

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

	assert((sizeof atom_names / sizeof atom_names[0]) == (sizeof(struct iron_x11_atoms) / sizeof(Atom)));
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
				pen->press = iron_internal_eraser_trigger_press;
				pen->move = iron_internal_eraser_trigger_move;
				pen->release = iron_internal_eraser_trigger_release;
			}
			else {
				pen->press = iron_internal_pen_trigger_press;
				pen->move = iron_internal_pen_trigger_move;
				pen->release = iron_internal_pen_trigger_release;
			}
			return;
		}
		c = (XAnyClassPtr)((uint8_t *)c + c->length);
	}
	xlib.XCloseDevice(x11_ctx.display, device);
}

static void check_pen_device(struct iron_x11_window *window, XEvent *event, struct x11_pen_device *pen) {
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

struct iron_x11_window *window_from_window(Window window) {
	for (int i = 0; i < 1; i++) {
		if (x11_ctx.windows[i].window == window) {
			return &x11_ctx.windows[i];
		}
	}
	return NULL;
}

void iron_vulkan_get_instance_extensions(const char **names, int *index) {
	names[(*index)++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
}

VkBool32 iron_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	return vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, x11_ctx.display,
														 DefaultVisual(x11_ctx.display, DefaultScreen(x11_ctx.display))->visualid);
}
VkResult iron_vulkan_create_surface(VkInstance instance, VkSurfaceKHR *surface) {
	VkXlibSurfaceCreateInfoKHR info = {0};
	info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.dpy = x11_ctx.display;
	info.window = x11_ctx.windows[0].window;
	return vkCreateXlibSurfaceKHR(instance, &info, NULL, surface);
}

static int xk_to_iron(KeySym symbol) {
	if (symbol == XK_Right) return IRON_KEY_RIGHT;
	if (symbol == XK_Left) return IRON_KEY_LEFT;
	if (symbol == XK_Up) return IRON_KEY_UP;
	if (symbol == XK_Down) return IRON_KEY_DOWN;
	if (symbol == XK_space) return IRON_KEY_SPACE;
	if (symbol == XK_BackSpace) return IRON_KEY_BACKSPACE;
	if (symbol == XK_Tab) return IRON_KEY_TAB;
	if (symbol == XK_Return) return IRON_KEY_RETURN;
	if (symbol == XK_Shift_L) return IRON_KEY_SHIFT;
	if (symbol == XK_Shift_R) return IRON_KEY_SHIFT;
	if (symbol == XK_Control_L) return IRON_KEY_CONTROL;
	if (symbol == XK_Control_R) return IRON_KEY_CONTROL;
	if (symbol == XK_Alt_L) return IRON_KEY_ALT;
	if (symbol == XK_Alt_R) return IRON_KEY_ALT;
	if (symbol == XK_Delete) return IRON_KEY_DELETE;
	if (symbol == XK_comma) return IRON_KEY_COMMA;
	if (symbol == XK_period) return IRON_KEY_PERIOD;
	if (symbol == XK_bracketleft) return IRON_KEY_OPEN_BRACKET;
	if (symbol == XK_bracketright) return IRON_KEY_CLOSE_BRACKET;
	if (symbol == XK_braceleft) return IRON_KEY_OPEN_CURLY_BRACKET;
	if (symbol == XK_braceright) return IRON_KEY_CLOSE_CURLY_BRACKET;
	if (symbol == XK_parenleft) return IRON_KEY_OPEN_PAREN;
	if (symbol == XK_parenright) return IRON_KEY_CLOSE_PAREN;
	if (symbol == XK_backslash) return IRON_KEY_BACK_SLASH;
	if (symbol == XK_apostrophe) return IRON_KEY_QUOTE;
	if (symbol == XK_colon) return IRON_KEY_COLON;
	if (symbol == XK_semicolon) return IRON_KEY_SEMICOLON;
	if (symbol == XK_minus) return IRON_KEY_HYPHEN_MINUS;
	if (symbol == XK_underscore) return IRON_KEY_UNDERSCORE;
	if (symbol == XK_slash) return IRON_KEY_SLASH;
	if (symbol == XK_bar) return IRON_KEY_PIPE;
	if (symbol == XK_question) return IRON_KEY_QUESTIONMARK;
	if (symbol == XK_less) return IRON_KEY_LESS_THAN;
	if (symbol == XK_greater) return IRON_KEY_GREATER_THAN;
	if (symbol == XK_asterisk) return IRON_KEY_ASTERISK;
	if (symbol == XK_ampersand) return IRON_KEY_AMPERSAND;
	if (symbol == XK_asciicircum) return IRON_KEY_CIRCUMFLEX;
	if (symbol == XK_percent) return IRON_KEY_PERCENT;
	if (symbol == XK_dollar) return IRON_KEY_DOLLAR;
	if (symbol == XK_numbersign) return IRON_KEY_HASH;
	if (symbol == XK_at) return IRON_KEY_AT;
	if (symbol == XK_exclam) return IRON_KEY_EXCLAMATION;
	if (symbol == XK_equal) return IRON_KEY_EQUALS;
	if (symbol == XK_plus) return IRON_KEY_ADD;
	if (symbol == XK_quoteleft) return IRON_KEY_BACK_QUOTE;
	if (symbol == XK_quotedbl) return IRON_KEY_DOUBLE_QUOTE;
	if (symbol == XK_asciitilde) return IRON_KEY_TILDE;
	if (symbol == XK_Pause) return IRON_KEY_PAUSE;
	if (symbol == XK_Scroll_Lock) return IRON_KEY_SCROLL_LOCK;
	if (symbol == XK_Home) return IRON_KEY_HOME;
	if (symbol == XK_Page_Up) return IRON_KEY_PAGE_UP;
	if (symbol == XK_Page_Down) return IRON_KEY_PAGE_DOWN;
	if (symbol == XK_End) return IRON_KEY_END;
	if (symbol == XK_Insert) return IRON_KEY_INSERT;
	if (symbol == XK_KP_Enter) return IRON_KEY_RETURN;
	if (symbol == XK_KP_Multiply) return IRON_KEY_MULTIPLY;
	if (symbol == XK_KP_Add) return IRON_KEY_ADD;
	if (symbol == XK_KP_Subtract) return IRON_KEY_SUBTRACT;
	if (symbol == XK_KP_Decimal) return IRON_KEY_DECIMAL;
	if (symbol == XK_KP_Divide) return IRON_KEY_DIVIDE;
	if (symbol == XK_KP_0) return IRON_KEY_NUMPAD_0;
	if (symbol == XK_KP_1) return IRON_KEY_NUMPAD_1;
	if (symbol == XK_KP_2) return IRON_KEY_NUMPAD_2;
	if (symbol == XK_KP_3) return IRON_KEY_NUMPAD_3;
	if (symbol == XK_KP_4) return IRON_KEY_NUMPAD_4;
	if (symbol == XK_KP_5) return IRON_KEY_NUMPAD_5;
	if (symbol == XK_KP_6) return IRON_KEY_NUMPAD_6;
	if (symbol == XK_KP_7) return IRON_KEY_NUMPAD_7;
	if (symbol == XK_KP_8) return IRON_KEY_NUMPAD_8;
	if (symbol == XK_KP_9) return IRON_KEY_NUMPAD_9;
	if (symbol == XK_KP_Insert) return IRON_KEY_INSERT;
	if (symbol == XK_KP_Delete) return IRON_KEY_DELETE;
	if (symbol == XK_KP_End) return IRON_KEY_END;
	if (symbol == XK_KP_Home) return IRON_KEY_HOME;
	if (symbol == XK_KP_Left) return IRON_KEY_LEFT;
	if (symbol == XK_KP_Up) return IRON_KEY_UP;
	if (symbol == XK_KP_Right) return IRON_KEY_RIGHT;
	if (symbol == XK_KP_Down) return IRON_KEY_DOWN;
	if (symbol == XK_KP_Page_Up) return IRON_KEY_PAGE_UP;
	if (symbol == XK_KP_Page_Down) return IRON_KEY_PAGE_DOWN;
	if (symbol == XK_Menu) return IRON_KEY_CONTEXT_MENU;
	if (symbol == XK_a) return IRON_KEY_A;
	if (symbol == XK_b) return IRON_KEY_B;
	if (symbol == XK_c) return IRON_KEY_C;
	if (symbol == XK_d) return IRON_KEY_D;
	if (symbol == XK_e) return IRON_KEY_E;
	if (symbol == XK_f) return IRON_KEY_F;
	if (symbol == XK_g) return IRON_KEY_G;
	if (symbol == XK_h) return IRON_KEY_H;
	if (symbol == XK_i) return IRON_KEY_I;
	if (symbol == XK_j) return IRON_KEY_J;
	if (symbol == XK_k) return IRON_KEY_K;
	if (symbol == XK_l) return IRON_KEY_L;
	if (symbol == XK_m) return IRON_KEY_M;
	if (symbol == XK_n) return IRON_KEY_N;
	if (symbol == XK_o) return IRON_KEY_O;
	if (symbol == XK_p) return IRON_KEY_P;
	if (symbol == XK_q) return IRON_KEY_Q;
	if (symbol == XK_r) return IRON_KEY_R;
	if (symbol == XK_s) return IRON_KEY_S;
	if (symbol == XK_t) return IRON_KEY_T;
	if (symbol == XK_u) return IRON_KEY_U;
	if (symbol == XK_v) return IRON_KEY_V;
	if (symbol == XK_w) return IRON_KEY_W;
	if (symbol == XK_x) return IRON_KEY_X;
	if (symbol == XK_y) return IRON_KEY_Y;
	if (symbol == XK_z) return IRON_KEY_Z;
	if (symbol == XK_1) return IRON_KEY_1;
	if (symbol == XK_2) return IRON_KEY_2;
	if (symbol == XK_3) return IRON_KEY_3;
	if (symbol == XK_4) return IRON_KEY_4;
	if (symbol == XK_5) return IRON_KEY_5;
	if (symbol == XK_6) return IRON_KEY_6;
	if (symbol == XK_7) return IRON_KEY_7;
	if (symbol == XK_8) return IRON_KEY_8;
	if (symbol == XK_9) return IRON_KEY_9;
	if (symbol == XK_0) return IRON_KEY_0;
	if (symbol == XK_Escape) return IRON_KEY_ESCAPE;
	if (symbol == XK_F1) return IRON_KEY_F1;
	if (symbol == XK_F2) return IRON_KEY_F2;
	if (symbol == XK_F3) return IRON_KEY_F3;
	if (symbol == XK_F4) return IRON_KEY_F4;
	if (symbol == XK_F5) return IRON_KEY_F5;
	if (symbol == XK_F6) return IRON_KEY_F6;
	if (symbol == XK_F7) return IRON_KEY_F7;
	if (symbol == XK_F8) return IRON_KEY_F8;
	if (symbol == XK_F9) return IRON_KEY_F9;
	if (symbol == XK_F10) return IRON_KEY_F10;
	if (symbol == XK_F11) return IRON_KEY_F11;
	if (symbol == XK_F12) return IRON_KEY_F12;
	return IRON_KEY_UNKNOWN;
}

static bool _handle_messages() {
	static bool controlDown = false;
	static int ignoreKeycode = 0;
	static bool preventNextKeyDownEvent = false;

	while (xlib.XPending(x11_ctx.display)) {
		XEvent event;
		xlib.XNextEvent(x11_ctx.display, &event);
		Window window = event.xclient.window;
		struct iron_x11_window *k_window = window_from_window(window);
		if (k_window == NULL) {
			continue;
		}
		check_pen_device(k_window, &event, &x11_ctx.pen);
		check_pen_device(k_window, &event, &x11_ctx.eraser);
		switch (event.type) {
		case MappingNotify: {
			xlib.XRefreshKeyboardMapping(&event.xmapping);
			break;
		}
		case KeyPress: {
			XKeyEvent *key = (XKeyEvent *)&event;
			KeySym keysym;
			wchar_t wchar;
			bool wcConverted = xlib.XwcLookupString(k_window->xInputContext, key, &wchar, 1, &keysym, NULL);

			bool isIgnoredKeySym = keysym == XK_Escape || keysym == XK_BackSpace || keysym == XK_Delete;
			if (!controlDown && !xlib.XFilterEvent(&event, window) && !isIgnoredKeySym) {
				if (wcConverted) {
					iron_internal_keyboard_trigger_key_press(wchar);
				}
			}

			if (preventNextKeyDownEvent) {
				// this keypress is a repeated keystroke and should not lead to a keydown-event
				preventNextKeyDownEvent = false;
				continue;
			}

			KeySym ksKey = xlib.XkbKeycodeToKeysym(x11_ctx.display, event.xkey.keycode, 0, 0);
			if (ksKey == XK_Control_L || ksKey == XK_Control_R) {
				controlDown = true;
			}
			else if (controlDown && (ksKey == XK_v || ksKey == XK_V)) {
				xlib.XConvertSelection(x11_ctx.display, x11_ctx.atoms.CLIPBOARD, x11_ctx.atoms.UTF8_STRING, x11_ctx.atoms.XSEL_DATA, window, CurrentTime);
			}
			else if (controlDown && (ksKey == XK_c || ksKey == XK_C)) {
				xlib.XSetSelectionOwner(x11_ctx.display, x11_ctx.atoms.CLIPBOARD, window, CurrentTime);
				char *text = iron_internal_copy_callback();
				if (text != NULL)
					iron_copy_to_clipboard(text);
			}
			else if (controlDown && (ksKey == XK_x || ksKey == XK_X)) {
				xlib.XSetSelectionOwner(x11_ctx.display, x11_ctx.atoms.CLIPBOARD, window, CurrentTime);
				char *text = iron_internal_cut_callback();
				if (text != NULL)
					iron_copy_to_clipboard(text);
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

			int key_code = xk_to_iron(ksKey);
			if (key_code != IRON_KEY_UNKNOWN) {
				iron_internal_keyboard_trigger_key_down(key_code);
			}

			break;
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

			int key_code = xk_to_iron(ksKey);
			if (key_code != IRON_KEY_UNKNOWN) {
				iron_internal_keyboard_trigger_key_up(key_code);
			}

			break;
		}
		case ButtonPress: {
			XButtonEvent *button = (XButtonEvent *)&event;

			switch (button->button) {
			case Button1:
				iron_internal_mouse_trigger_press(0, button->x, button->y);
				break;
			case Button2:
				iron_internal_mouse_trigger_press(2, button->x, button->y);
				break;
			case Button3:
				iron_internal_mouse_trigger_press(1, button->x, button->y);
				break;
			// buttons 4-7 are for mouse wheel events because why not
			case Button4:
			case Button5:
			case Button6:
			case Button7:
				break;
			default:
				iron_internal_mouse_trigger_press(button->button - Button1 - 4, button->x, button->y);
				break;
			}
			break;
		}
		case ButtonRelease: {
			XButtonEvent *button = (XButtonEvent *)&event;

			switch (button->button) {
			case Button1:
				iron_internal_mouse_trigger_release(0, button->x, button->y);
				break;
			case Button2:
				iron_internal_mouse_trigger_release(2, button->x, button->y);
				break;
			case Button3:
				iron_internal_mouse_trigger_release(1, button->x, button->y);
				break;
			// Button4 and Button5 provide mouse wheel events because why not
			case Button4:
				iron_internal_mouse_trigger_scroll(-1);
				break;
			case Button5:
				iron_internal_mouse_trigger_scroll(1);
				break;
			// button 6 and 7 seem to be horizontal scrolling, which is not exposed in Iron's api at the moment
			case Button6:
			case Button7:
				break;
			default:
				iron_internal_mouse_trigger_release(button->button - Button1 - 4, button->x, button->y);
				break;
			}
			break;
		}
		case MotionNotify: {
			XMotionEvent *motion = (XMotionEvent *)&event;
			iron_internal_mouse_trigger_move(motion->x, motion->y);
			break;
		}
		case ConfigureNotify: {
			if (event.xconfigure.width != k_window->width || event.xconfigure.height != k_window->height) {
				k_window->width = event.xconfigure.width;
				k_window->height = event.xconfigure.height;
				gpu_resize(event.xconfigure.width, event.xconfigure.height);
				iron_internal_call_resize_callback(event.xconfigure.width, event.xconfigure.height);
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
				if (iron_internal_call_close_callback()) {
					iron_window_destroy();
					iron_stop();
				}
			}
			break;
		}
		case SelectionNotify: {
			if (event.xselection.selection == x11_ctx.atoms.CLIPBOARD) {
				char *result;
				unsigned long ressize, restail;
				int resbits;
				xlib.XGetWindowProperty(x11_ctx.display, window, x11_ctx.atoms.XSEL_DATA, 0, LONG_MAX / 4, False, AnyPropertyType, &x11_ctx.atoms.UTF8_STRING,
										&resbits, &ressize, &restail, (unsigned char **)&result);
				iron_internal_paste_callback(result);
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
						iron_internal_drop_files_callback(filePath + 7); // Strip file://
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
		case Expose: {
			break;
		}
		case FocusIn: {
			iron_internal_foreground_callback();
			break;
		}
		case FocusOut: {
			controlDown = false;
			ignoreKeycode = 0;
			iron_internal_background_callback();
			break;
		}
		case LeaveNotify: {
			break;
		}
		case EnterNotify: {
			break;
		}
		}
	}

	return true;
}

bool iron_internal_handle_messages() {
	if (!_handle_messages()) {
		return false;
	}
	#ifdef WITH_GAMEPAD
	iron_linux_updateHIDGamepads();
	#endif
	return true;
}

const char *iron_system_id() {
	return "Linux";
}

void iron_set_keep_screen_on(bool on) {}

void iron_keyboard_show() {}

void iron_keyboard_hide() {}

bool iron_keyboard_active() {
	return true;
}

void iron_load_url(const char *url) {
	if (strncmp(url, "http://", sizeof("http://") - 1) == 0 || strncmp(url, "https://", sizeof("https://") - 1) == 0) {
		char openUrlCommand[256];
		snprintf(openUrlCommand, 256, "xdg-open %s", url);
		int err = system(openUrlCommand);
		if (err != 0) {
			iron_log("Error opening url %s", url);
		}
	}
}

const char *iron_language() {
	return "en";
}

const char *iron_internal_save_path() {
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
		strcat(save, iron_application_name());
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
			strcat(save, iron_application_name());
			strcat(save, "/");
			int res = mkdir(save, 0700);
			if (res != 0 && errno != EEXIST) {
				iron_error("Could not create save directory '%s'. Error %d", save, errno);
			}
		}
		saveInitialized = true;
	}
	return save;
}

const char **iron_video_formats() {
	return videoFormats;
}

double iron_frequency(void) {
	return 1000000.0;
}

uint64_t iron_timestamp(void) {
	struct timeval now;
	gettimeofday(&now, NULL);
	now.tv_sec -= start.tv_sec;
	now.tv_usec -= start.tv_usec;
	return (uint64_t)now.tv_sec * 1000000 + (uint64_t)now.tv_usec;
}

void iron_init(iron_window_options_t *win) {
	gettimeofday(&start, NULL);
	#ifdef WITH_GAMEPAD
	iron_linux_initHIDGamepads();
	#endif
	iron_x11_init();
	iron_display_init();
	iron_set_app_name(win->title);
	iron_window_create(win);
}

void iron_internal_shutdown() {
	gpu_destroy();

	#ifdef WITH_GAMEPAD
	iron_linux_closeHIDGamepads();
	#endif

	free(clipboardString);
	xlib.XCloseDisplay(x11_ctx.display);
	iron_internal_shutdown_callback();
}

#ifndef IRON_NO_MAIN
int main(int argc, char **argv) {
	return kickstart(argc, argv);
}
#endif

void iron_copy_to_clipboard(const char *text) {
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

int iron_hardware_threads(void) {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void iron_internal_mouse_lock() {
	iron_mouse_hide();
	int width = iron_window_width();
	int height = iron_window_height();

	int x, y;
	iron_mouse_get_position(&x, &y);
	int newX = x;
	int newY = y;
	if (x < 0) {
		newX -= x;
	}
	else if (x > width) {
		newX -= x - width;
	}
	if (y < 0) {
		newY -= y;
	}
	else if (y > height) {
		newY -= y - height;
	}
	iron_mouse_set_position(newX, newY);
}

void iron_internal_mouse_unlock() {
	iron_mouse_show();
}

bool iron_mouse_can_lock(void) {
	return true;
}

void iron_mouse_show() {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	if (mouse_hidden) {
		xlib.XUndefineCursor(x11_ctx.display, window->window);
		mouse_hidden = false;
	}
}

void iron_mouse_hide() {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	if (!mouse_hidden) {
		XColor col;
		col.pixel = 0;
		col.red = 0;
		col.green = 0;
		col.blue = 0;
		col.flags = DoRed | DoGreen | DoBlue;
		col.pad = 0;
		char data[1] = {'\0'};
		Pixmap blank = xlib.XCreateBitmapFromData(x11_ctx.display, window->window, data, 1, 1);
		Cursor cursor = xlib.XCreatePixmapCursor(x11_ctx.display, blank, blank, &col, &col, 0, 0);
		xlib.XDefineCursor(x11_ctx.display, window->window, cursor);
		xlib.XFreePixmap(x11_ctx.display, blank);
		mouse_hidden = true;
	}
}

void iron_mouse_set_cursor(int cursor_index) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	if (!mouse_hidden) {
		Cursor cursor;
		if (cursor_index == 0) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "left_ptr"); // "arrow"
		}
		else if (cursor_index == 1) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "hand1");
		}
		else if (cursor_index == 2) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "xterm");
		}
		else if (cursor_index == 3) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "sb_h_double_arrow");
		}
		else if (cursor_index == 4) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "sb_v_double_arrow");
		}
		else if (cursor_index == 5) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "top_right_corner");
		}
		else if (cursor_index == 6) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "bottom_right_corner");
		}
		else if (cursor_index == 7) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "top_left_corner");
		}
		else if (cursor_index == 8) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "bottom_left_corner");
		}
		else if (cursor_index == 9) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "grab");
		}
		else if (cursor_index == 10) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "grabbing");
		}
		else if (cursor_index == 11) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "not-allowed");
		}
		else if (cursor_index == 12) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "watch");
		}
		else if (cursor_index == 13) {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "crosshair");
		}
		else {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "arrow");
		}
		xlib.XDefineCursor(x11_ctx.display, window->window, cursor);
	}
}

void iron_mouse_set_position(int x, int y) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	xlib.XWarpPointer(x11_ctx.display, None, window->window, 0, 0, 0, 0, x, y);
	xlib.XFlush(x11_ctx.display);
}

void iron_mouse_get_position(int *x, int *y) {
	struct iron_x11_window *window = &x11_ctx.windows[0];
	Window inwin;
	Window inchildwin;
	int rootx, rooty;
	unsigned int mask;
	xlib.XQueryPointer(x11_ctx.display, window->window, &inwin, &inchildwin, &rootx, &rooty, x, y, &mask);
}

#ifdef WITH_GAMEPAD

#include <fcntl.h>
#include <libudev.h>
#include <linux/joystick.h>

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
		iron_internal_gamepad_trigger_connect(pad->idx);
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
		iron_internal_gamepad_trigger_disconnect(pad->idx);
		close(pad->file_descriptor);
		pad->file_descriptor = -1;
		pad->connected = false;
	}
}

void HIDGamepad_processEvent(struct HIDGamepad *pad, struct js_event e) {
	switch (e.type) {
	case JS_EVENT_BUTTON:
		iron_internal_gamepad_trigger_button(pad->idx, e.number, e.value);
		break;
	case JS_EVENT_AXIS: {
		float value = e.number % 2 == 0 ? e.value : -e.value;
		iron_internal_gamepad_trigger_axis(pad->idx, e.number, value / 32767.0f);
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

static struct HIDGamepad gamepads[IRON_GAMEPAD_MAX_COUNT];

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

void iron_linux_initHIDGamepads() {
	for (int i = 0; i < IRON_GAMEPAD_MAX_COUNT; ++i) {
		HIDGamepad_init(&gamepads[i], i);
	}
	HIDGamepadUdevHelper_init(&udev_helper);
}

void iron_linux_updateHIDGamepads() {
	HIDGamepadUdevHelper_update(&udev_helper);
	for (int i = 0; i < IRON_GAMEPAD_MAX_COUNT; ++i) {
		HIDGamepad_update(&gamepads[i]);
	}
}

void iron_linux_closeHIDGamepads() {
	HIDGamepadUdevHelper_close(&udev_helper);
}

const char *iron_gamepad_vendor(int gamepad) {
	return "Linux gamepad";
}

const char *iron_gamepad_product_name(int gamepad) {
	return gamepad >= 0 && gamepad < IRON_GAMEPAD_MAX_COUNT ? gamepads[gamepad].name : "";
}

bool iron_gamepad_connected(int gamepad) {
	return gamepad >= 0 && gamepad < IRON_GAMEPAD_MAX_COUNT && gamepads[gamepad].connected;
}

void iron_gamepad_rumble(int gamepad, float left, float right) {}

#endif
