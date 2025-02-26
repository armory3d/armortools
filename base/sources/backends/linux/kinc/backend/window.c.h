#include <iron_gpu.h>
#include <iron_system.h>
#include <string.h>
#include <stdlib.h>
#include "x11.h"

struct MwmHints {
	// These correspond to XmRInt resources. (VendorSE.c)
	int flags;
	int functions;
	int decorations;
	int input_mode;
	int status;
};
#define MWM_HINTS_DECORATIONS (1L << 1)

int kinc_window_x() {
	return 0;
}

int kinc_window_y() {
	return 0;
}

int kinc_window_width() {
	return x11_ctx.windows[0].width;
}

int kinc_window_height() {
	return x11_ctx.windows[0].height;
}

void kinc_window_resize(int width, int height) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	xlib.XResizeWindow(x11_ctx.display, window->window, width, height);
}

void kinc_window_move(int x, int y) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	xlib.XMoveWindow(x11_ctx.display, window->window, x, y);
}

void kinc_window_change_features(int features) {}

void kinc_window_change_mode(kinc_window_mode_t mode) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	if (mode == window->mode) {
		return;
	}

	bool fullscreen = false;

	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW:
		if (window->mode == KINC_WINDOW_MODE_FULLSCREEN) {
			window->mode = KINC_WINDOW_MODE_WINDOW;
			fullscreen = false;
		}
		else {
			return;
		}
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
		if (window->mode == KINC_WINDOW_MODE_WINDOW) {
			window->mode = KINC_WINDOW_MODE_FULLSCREEN;
			fullscreen = true;
		}
		else {
			return;
		}
		break;
	}

	XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = window->window;
	xev.xclient.message_type = x11_ctx.atoms.NET_WM_STATE;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = fullscreen ? 1 : 0;
	xev.xclient.data.l[1] = x11_ctx.atoms.NET_WM_STATE_FULLSCREEN;
	xev.xclient.data.l[2] = 0;

	xlib.XMapWindow(x11_ctx.display, window->window);

	xlib.XSendEvent(x11_ctx.display, DefaultRootWindow(x11_ctx.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

	xlib.XFlush(x11_ctx.display);
}

int kinc_window_display() {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	return window->display_index;
}

void kinc_window_destroy() {
	kinc_g5_internal_destroy_window();

	xlib.XFlush(x11_ctx.display);
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	xlib.XDestroyIC(window->xInputContext);
	xlib.XCloseIM(window->xInputMethod);
	xlib.XDestroyWindow(x11_ctx.display, window->window);
	xlib.XFlush(x11_ctx.display);
	*window = (struct kinc_x11_window){0};
}

void kinc_window_show() {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	xlib.XMapWindow(x11_ctx.display, window->window);
}

void kinc_window_hide() {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	xlib.XUnmapWindow(x11_ctx.display, window->window);
}

void kinc_window_set_title(const char *_title) {
	const char *title = _title == NULL ? "" : _title;
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	xlib.XChangeProperty(x11_ctx.display, window->window, x11_ctx.atoms.NET_WM_NAME, x11_ctx.atoms.UTF8_STRING, 8, PropModeReplace, (unsigned char *)title,
	                     strlen(title));

	xlib.XChangeProperty(x11_ctx.display, window->window, x11_ctx.atoms.NET_WM_ICON_NAME, x11_ctx.atoms.UTF8_STRING, 8, PropModeReplace, (unsigned char *)title,
	                     strlen(title));

	xlib.XFlush(x11_ctx.display);
}

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	window->width = win->width;
	window->height = win->height;

	Visual *visual = NULL;
	XSetWindowAttributes set_window_attribs = {0};

	set_window_attribs.border_pixel = 0;
	set_window_attribs.event_mask =
	    KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;
	int screen = DefaultScreen(x11_ctx.display);
	visual = DefaultVisual(x11_ctx.display, screen);
	int depth = DefaultDepth(x11_ctx.display, screen);
	set_window_attribs.colormap = xlib.XCreateColormap(x11_ctx.display, RootWindow(x11_ctx.display, screen), visual, AllocNone);
	window->window = xlib.XCreateWindow(x11_ctx.display, RootWindow(x11_ctx.display, DefaultScreen(x11_ctx.display)), 0, 0, win->width, win->height, 0, depth,
	                                    InputOutput, visual, CWBorderPixel | CWColormap | CWEventMask, &set_window_attribs);

	static char nameClass[256];
	static const char *nameClassAddendum = "_KincApplication";
	strncpy(nameClass, kinc_application_name(), sizeof(nameClass) - strlen(nameClassAddendum) - 1);
	strcat(nameClass, nameClassAddendum);
	char resNameBuffer[256];
	strncpy(resNameBuffer, kinc_application_name(), 256);
	XClassHint classHint = {.res_name = resNameBuffer, .res_class = nameClass};
	xlib.XSetClassHint(x11_ctx.display, window->window, &classHint);

	xlib.XSetLocaleModifiers("@im=none");
	window->xInputMethod = xlib.XOpenIM(x11_ctx.display, NULL, NULL, NULL);
	window->xInputContext = xlib.XCreateIC(window->xInputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window->window, NULL);
	xlib.XSetICFocus(window->xInputContext);

	window->mode = KINC_WINDOW_MODE_WINDOW;
	kinc_window_change_mode(win->mode);

	xlib.XMapWindow(x11_ctx.display, window->window);

	Atom XdndVersion = 5;
	xlib.XChangeProperty(x11_ctx.display, window->window, x11_ctx.atoms.XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char *)&XdndVersion, 1);
	xlib.XSetWMProtocols(x11_ctx.display, window->window, &x11_ctx.atoms.WM_DELETE_WINDOW, 1);

	kinc_window_set_title(win->title);

	if (x11_ctx.pen.id != -1) {
		xlib.XSelectExtensionEvent(x11_ctx.display, window->window, &x11_ctx.pen.motionClass, 1);
	}

	if (x11_ctx.eraser.id != -1) {
		xlib.XSelectExtensionEvent(x11_ctx.display, window->window, &x11_ctx.eraser.motionClass, 1);
	}

	kinc_g4_internal_init_window(frame->depth_bits, frame->vertical_sync);
}

static struct {
	void (*resize_callback)(int width, int height, void *data);
	void *resize_data;
	void *ppi_data;
	bool (*close_callback)(void *data);
	void *close_data;
} kinc_internal_window_callbacks[16];

void kinc_window_set_resize_callback(void (*callback)(int width, int height, void *data), void *data) {
	kinc_internal_window_callbacks[0].resize_callback = callback;
	kinc_internal_window_callbacks[0].resize_data = data;
}

void kinc_internal_call_resize_callback(int width, int height) {
	if (kinc_internal_window_callbacks[0].resize_callback != NULL) {
		kinc_internal_window_callbacks[0].resize_callback(width, height, kinc_internal_window_callbacks[0].resize_data);
	}
}

void kinc_window_set_close_callback(bool (*callback)(void *data), void *data) {
	kinc_internal_window_callbacks[0].close_callback = callback;
	kinc_internal_window_callbacks[0].close_data = data;
}

bool kinc_internal_call_close_callback() {
	if (kinc_internal_window_callbacks[0].close_callback != NULL) {
		return kinc_internal_window_callbacks[0].close_callback(kinc_internal_window_callbacks[0].close_data);
	}
	else {
		return true;
	}
}

kinc_window_mode_t kinc_window_get_mode() {
	return x11_ctx.windows[0].mode;
}
