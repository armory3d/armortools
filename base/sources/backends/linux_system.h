#pragma once

#include <iron_system.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>

#define MAXIMUM_DISPLAYS 8

struct iron_x11_window {
	int display_index;
	int width;
	int height;
	iron_window_mode_t mode;
	Window window;
	XIM xInputMethod;
	XIC xInputContext;
};

struct iron_x11_display {
	int index;
	int x;
	int y;
	int width;
	int height;
	bool primary;
	RROutput output;
	RRCrtc crtc;
};

struct x11_pen_device {
	int id;
	int pressure_index;
	double pressure_max;
	float pressure_current;
	void (*press)(int /*x*/, int /*y*/, float /*pressure*/);
	void (*move)(int /*x*/, int /*y*/, float /*pressure*/);
	void (*release)(int /*x*/, int /*y*/, float /*pressure*/);
};

struct x11_context {
	Display *display;
	struct x11_pen_device pen;
	struct x11_pen_device eraser;
	struct iron_x11_window windows[1];
	int num_displays;
	struct iron_x11_display displays[MAXIMUM_DISPLAYS];
};

#ifdef WITH_GAMEPAD
void iron_linux_initHIDGamepads();
void iron_linux_updateHIDGamepads();
void iron_linux_closeHIDGamepads();
#endif
