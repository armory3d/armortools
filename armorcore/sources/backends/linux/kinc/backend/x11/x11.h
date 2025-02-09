#pragma once

#include <kinc/display.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xrandr.h>

#define MAXIMUM_WINDOWS 16
#define MAXIMUM_DISPLAYS 16

struct kinc_x11_window {
	int display_index;
	int window_index;
	int width;
	int height;
	kinc_window_mode_t mode;
	Window window;
	XIM xInputMethod;
	XIC xInputContext;
};

struct kinc_x11_display {
	int index;
	int current_mode;
	int num_modes;
	int x;
	int y;
	int width;
	int height;
	bool primary;
	char name[64];

	RROutput output;
	RRCrtc crtc;
};

struct kinc_x11_mouse {
	int current_window;
	int x;
	int y;
};

struct kinc_x11_atoms {
	Atom XdndAware;
	Atom XdndDrop;
	Atom XdndEnter;
	Atom XdndTextUriList;
	Atom XdndStatus;
	Atom XdndActionCopy;
	Atom XdndSelection;
	Atom CLIPBOARD;
	Atom UTF8_STRING;
	Atom XSEL_DATA;
	Atom TARGETS;
	Atom MULTIPLE;
	Atom TEXT_PLAIN;
	Atom WM_DELETE_WINDOW;
	Atom MOTIF_WM_HINTS;
	Atom NET_WM_NAME;
	Atom NET_WM_ICON_NAME;
	Atom NET_WM_STATE;
	Atom NET_WM_STATE_FULLSCREEN;

	Atom MOUSE;
	Atom TABLET;
	Atom KEYBOARD;
	Atom TOUCHSCREEN;
	Atom TOUCHPAD;
	Atom BUTTONBOX;
	Atom BARCODE;
	Atom TRACKBALL;
	Atom QUADRATURE;
	Atom ID_MODULE;
	Atom ONE_KNOB;
	Atom NINE_KNOB;
	Atom KNOB_BOX;
	Atom SPACEBALL;
	Atom DATAGLOVE;
	Atom EYETRACKER;
	Atom CURSORKEYS;
	Atom FOOTMOUSE;
	Atom JOYSTICK;
};

struct kinc_x11_libs {
	void *X11;
	void *Xcursor;
	void *Xi;
	void *Xinerama;
	void *Xrandr;
};

struct kinc_x11_procs {
	Display *(*XOpenDisplay)(const char *name);
	Status (*XInternAtoms)(Display *display, char **names, int count, Bool only_if_exists, Atom *atoms_return);
	int (*XCloseDisplay)(Display *display);
	XErrorHandler (*XSetErrorHandler)(XErrorHandler handler);
	int (*XGetErrorText)(Display *, int, char *, int);
	int (*XPending)(Display *display);
	int (*XFlush)(Display *display);
	int (*XNextEvent)(Display *display, XEvent *event_return);
	int (*XPeekEvent)(Display *display, XEvent *event_return);
	int (*XRefreshKeyboardMapping)(XMappingEvent *event_map);
	int (*XwcLookupString)(XIC, XKeyPressedEvent *, wchar_t *, int, KeySym *, int *);
	int (*XFilterEvent)(XEvent *, Window);
	int (*XConvertSelection)(Display *, Atom, Atom, Atom, Window, Time);
	int (*XSetSelectionOwner)(Display *, Atom, Window, Time);
	int (*XLookupString)(XKeyEvent *, char *, int, KeySym *, XComposeStatus *);
	KeySym (*XkbKeycodeToKeysym)(Display *, KeyCode, int, int);
	int (*XSendEvent)(Display *, Window, int, long, XEvent *);
	int (*XGetWindowProperty)(Display *, Window, Atom, long, long, int, Atom, Atom *, int *, unsigned long *, unsigned long *, unsigned char **);
	int (*XFree)(void *);
	int (*XChangeProperty)(Display *, Window, Atom, Atom, int, int, const unsigned char *, int);
	int (*XDefineCursor)(Display *, Window, Cursor);
	int (*XUndefineCursor)(Display *, Window);
	Pixmap (*XCreateBitmapFromData)(Display *, Drawable, const char *, unsigned int, unsigned int);
	Cursor (*XCreatePixmapCursor)(Display *, Pixmap, Pixmap, XColor *, XColor *, unsigned int, unsigned int);
	int (*XFreePixmap)(Display *, Pixmap);
	Cursor (*XcursorLibraryLoadCursor)(Display *, const char *);
	int (*XWarpPointer)(Display *, Window, Window, int, int, unsigned int, unsigned int, int, int);
	int (*XQueryPointer)(Display *, Window, Window *, Window *, int *, int *, int *, int *, unsigned int *);
	Colormap (*XCreateColormap)(Display *, Window, Visual *, int);
	Window (*XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth,
	                        unsigned int class, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes);
	int (*XMoveWindow)(Display *, Window, int, int);
	int (*XResizeWindow)(Display *, Window, unsigned int, unsigned int);
	int (*XDestroyWindow)(Display *, Window);
	int (*XSetClassHint)(Display *, Window, XClassHint *);
	char *(*XSetLocaleModifiers)(const char *);
	XIM (*XOpenIM)(Display *, struct _XrmHashBucketRec *, char *, char *);
	int (*XCloseIM)(XIM);
	XIC (*XCreateIC)(XIM, ...);
	void (*XDestroyIC)(XIC);
	void (*XSetICFocus)(XIC);
	int (*XMapWindow)(Display *, Window);
	int (*XUnmapWindow)(Display *, Window);
	int (*XSetWMProtocols)(Display *, Window, Atom *, int);

	XDeviceInfo *(*XListInputDevices)(Display *, int *);
	void (*XFreeDeviceList)(XDeviceInfo *);
	XDevice *(*XOpenDevice)(Display *display, XID device_id);
	int (*XCloseDevice)(Display *display, XDevice *device);
	int (*XSelectExtensionEvent)(Display *, Window, XEventClass *, int);

	int (*XineramaQueryExtension)(Display *dpy, int *event_base, int *error_base);
	int (*XineramaIsActive)(Display *dpy);
	XineramaScreenInfo *(*XineramaQueryScreens)(Display *dpy, int *number);

	XRRScreenResources *(*XRRGetScreenResourcesCurrent)(Display *dpy, Window window);
	RROutput (*XRRGetOutputPrimary)(Display *dpy, Window window);
	XRROutputInfo *(*XRRGetOutputInfo)(Display *dpy, XRRScreenResources *resources, RROutput output);
	void (*XRRFreeOutputInfo)(XRROutputInfo *outputInfo);
	XRRCrtcInfo *(*XRRGetCrtcInfo)(Display *dpy, XRRScreenResources *resources, RRCrtc crtc);
	void (*XRRFreeCrtcInfo)(XRRCrtcInfo *crtcInfo);
	void (*XRRFreeScreenResources)(XRRScreenResources *resources);
};

struct x11_pen_device {
	XID id;
	uint32_t motionEvent;
	XEventClass motionClass;
	uint32_t maxPressure;
	float current_pressure;

	void (*press)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
	void (*move)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
	void (*release)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/);
};

struct x11_context {
	Display *display;
	struct kinc_x11_libs libs;
	struct kinc_x11_atoms atoms;
	struct kinc_x11_mouse mouse;

	struct x11_pen_device pen;
	struct x11_pen_device eraser;

	int num_windows;
	struct kinc_x11_window windows[MAXIMUM_WINDOWS];
	int num_displays;
	struct kinc_x11_display displays[MAXIMUM_DISPLAYS];
};

struct kinc_x11_procs xlib;
struct x11_context x11_ctx;

void kinc_x11_copy_to_clipboard(const char *text);