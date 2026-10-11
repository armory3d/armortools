// Terminal window placed next to the pad window

bool terminal_show = false;

#ifdef IRON_LINUX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <signal.h>
#include <sys/wait.h>

#define TERMINAL_SPLIT 0.7 // Pad takes the left part of the screen

static Display *terminal_display        = NULL; // Own connection, the one of the window is internal to iron
static Window   terminal_pad            = 0;
static pid_t    terminal_pid            = 0;
static Window   terminal_window         = 0; // Placed once the window shows up
static i32      terminal_settle         = 0; // Frames until the windows are placed again
static i32      terminal_pad_x          = 0; // Pad window before the split
static i32      terminal_pad_y          = 0;
static i32      terminal_pad_w          = 0;
static i32      terminal_pad_h          = 0;
static bool     terminal_pad_maximized  = false;
static bool     terminal_pad_fullscreen = false;

static Display *terminal_x11() {
	if (terminal_display == NULL) {
		terminal_display = XOpenDisplay(NULL);
	}
	return terminal_display;
}

// Reads up to count values of a window property, returns how many were read
static i32 terminal_get_prop(Window window, char *name, long *values, i32 count) {
	Display       *display = terminal_x11();
	Atom           type;
	int            format;
	unsigned long  length;
	unsigned long  left;
	unsigned char *data = NULL;
	if (XGetWindowProperty(display, window, XInternAtom(display, name, False), 0, count, False, AnyPropertyType, &type, &format, &length, &left, &data) !=
	        Success ||
	    data == NULL) {
		return 0;
	}
	i32 n = format == 32 ? (i32)length : 0;
	for (i32 i = 0; i < n; ++i) {
		values[i] = ((long *)data)[i];
	}
	XFree(data);
	return n;
}

// Screen without the desktop panels
static void terminal_work_area(i32 *x, i32 *y, i32 *w, i32 *h) {
	Display *display = terminal_x11();
	Window   root    = DefaultRootWindow(display);
	long     desktop = 0;
	long     areas[64];
	terminal_get_prop(root, "_NET_CURRENT_DESKTOP", &desktop, 1);
	i32 n = terminal_get_prop(root, "_NET_WORKAREA", areas, 64);
	if (desktop < 0 || desktop * 4 + 3 >= n) {
		*x = 0;
		*y = 0;
		*w = sys_display_width();
		*h = sys_display_height();
		return;
	}
	*x = areas[desktop * 4];
	*y = areas[desktop * 4 + 1];
	*w = areas[desktop * 4 + 2];
	*h = areas[desktop * 4 + 3];
}

// Moves the window so its frame covers the rect, extents are left, right, top and bottom
static void terminal_place(Window window, long *extents, i32 x, i32 y, i32 w, i32 h) {
	XMoveResizeWindow(terminal_x11(), window, x, y, w - extents[0] - extents[1], h - extents[2] - extents[3]);
}

static void terminal_set_maximized(Window window, bool maximized) {
	Display *display = terminal_x11();
	XEvent   xev;
	memset(&xev, 0, sizeof(xev));
	xev.type                 = ClientMessage;
	xev.xclient.window       = window;
	xev.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
	xev.xclient.format       = 32;
	xev.xclient.data.l[0]    = maximized ? 1 : 0;
	xev.xclient.data.l[1]    = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	xev.xclient.data.l[2]    = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	xev.xclient.data.l[3]    = 1;
	XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

static bool terminal_is_maximized(Window window) {
	Display *display = terminal_x11();
	long     states[16];
	i32      n = terminal_get_prop(window, "_NET_WM_STATE", states, 16);
	for (i32 i = 0; i < n; ++i) {
		if ((Atom)states[i] == XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False)) {
			return true;
		}
	}
	return false;
}

// Top level window of the terminal process
static Window terminal_find_window() {
	Display *display = terminal_x11();
	long     clients[1024];
	i32      n = terminal_get_prop(DefaultRootWindow(display), "_NET_CLIENT_LIST", clients, 1024);
	for (i32 i = 0; i < n; ++i) {
		long pid = 0;
		if (terminal_get_prop(clients[i], "_NET_WM_PID", &pid, 1) == 1 && pid == terminal_pid) {
			return clients[i];
		}
	}
	return 0;
}

// Pad takes the left part of the screen, the terminal gets the rest
static void terminal_layout() {
	Display *display    = terminal_x11();
	long     extents[4] = {0, 0, 0, 0};
	i32      x, y, w, h;
	terminal_work_area(&x, &y, &w, &h);
	i32 pad_w = math_floor(w * TERMINAL_SPLIT);
	terminal_get_prop(terminal_pad, "_NET_FRAME_EXTENTS", extents, 4);
	terminal_place(terminal_pad, extents, x, y, pad_w, h);
	if (terminal_window != 0) {
		XSizeHints hints;
		long       supplied;
		if (XGetWMNormalHints(display, terminal_window, &hints, &supplied) != 0 && (hints.flags & PResizeInc) != 0) {
			hints.flags &= ~PResizeInc; // Fill the rect, not just whole character cells
			XSetWMNormalHints(display, terminal_window, &hints);
		}
		terminal_get_prop(terminal_window, "_NET_FRAME_EXTENTS", extents, 4);
		terminal_place(terminal_window, extents, x + pad_w, y, w - pad_w, h);
	}
	XFlush(display);
}

static bool terminal_exited() {
	if (terminal_pid > 0 && waitpid(terminal_pid, NULL, WNOHANG) != 0) {
		terminal_pid = 0;
	}
	return terminal_pid == 0;
}

static void terminal_open() {
	Display *display = terminal_x11();
	long     active  = 0;
	if (display == NULL || terminal_get_prop(DefaultRootWindow(display), "_NET_ACTIVE_WINDOW", &active, 1) != 1 || active == 0) {
		return;
	}
	terminal_pad      = active; // Pad has the focus, the shortcut was pressed in it
	Window window     = terminal_pad;
	long   extents[4] = {0, 0, 0, 0};
	Window child;
	terminal_get_prop(window, "_NET_FRAME_EXTENTS", extents, 4);
	XTranslateCoordinates(display, window, DefaultRootWindow(display), 0, 0, &terminal_pad_x, &terminal_pad_y, &child);
	terminal_pad_x -= extents[0];
	terminal_pad_y -= extents[2];
	terminal_pad_w          = iron_window_width() + extents[0] + extents[1];
	terminal_pad_h          = iron_window_height() + extents[2] + extents[3];
	terminal_pad_maximized  = terminal_is_maximized(window);
	terminal_pad_fullscreen = iron_window_get_mode() == IRON_WINDOW_MODE_FULLSCREEN;
	if (terminal_pad_fullscreen) {
		iron_window_change_mode(IRON_WINDOW_MODE_WINDOW);
	}
	if (terminal_pad_maximized) {
		terminal_set_maximized(window, false);
	}
	terminal_window = 0;
	terminal_settle = 0;
	terminal_layout();

	terminal_pid = fork();
	if (terminal_pid == 0) {
		if (pad_project_dir != NULL && chdir(pad_project_dir) != 0) {
			_exit(127);
		}
		char *terminal = getenv("TERMINAL");
		if (terminal != NULL && terminal[0] != '\0') {
			execlp(terminal, terminal, (char *)NULL);
		}
		execlp("xfce4-terminal", "xfce4-terminal", "--disable-server", (char *)NULL); // Own process, exits with the window
		_exit(127);
	}
	terminal_show = terminal_pid > 0;
}

// Pad window goes back to where it was
static void terminal_restore() {
	Display *display    = terminal_x11();
	Window   window     = terminal_pad;
	long     extents[4] = {0, 0, 0, 0};
	terminal_get_prop(window, "_NET_FRAME_EXTENTS", extents, 4);
	terminal_place(window, extents, terminal_pad_x, terminal_pad_y, terminal_pad_w, terminal_pad_h);
	if (terminal_pad_maximized) {
		terminal_set_maximized(window, true);
	}
	if (terminal_pad_fullscreen) {
		iron_window_change_mode(IRON_WINDOW_MODE_FULLSCREEN);
	}
	XFlush(display);
	terminal_show = false;
}

void terminal_toggle() {
	if (!terminal_show) {
		terminal_open();
	}
	else if (terminal_pid > 0) {
		kill(terminal_pid, SIGTERM); // Restored in terminal_update
	}
}

void terminal_update() {
	if (!terminal_show) {
		return;
	}
	if (terminal_exited()) { // Closed from here or from the terminal
		terminal_restore();
		return;
	}
	if (terminal_window == 0) {
		terminal_window = terminal_find_window();
		if (terminal_window != 0) {
			terminal_layout(); // Pad again, its frame differs when it was maximized
			terminal_settle = 10;
		}
	}
	else if (terminal_settle > 0 && --terminal_settle == 0) {
		terminal_layout(); // Window manager has the new size hints by now
	}
}

// Keeps the frames coming until the terminal is placed, and once it is closed
bool terminal_idle_wake() {
	return terminal_show && (terminal_window == 0 || terminal_settle > 0 || terminal_exited());
}

void terminal_shutdown() {
	if (terminal_pid > 0) {
		kill(terminal_pid, SIGTERM);
	}
}

#else

void terminal_toggle() {}

void terminal_update() {}

bool terminal_idle_wake() {
	return false;
}

void terminal_shutdown() {}

#endif
