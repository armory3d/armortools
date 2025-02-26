#include <iron_input.h>

static bool mouse_hidden = false;

void kinc_internal_mouse_lock() {
	kinc_mouse_hide();
	int width = kinc_window_width();
	int height = kinc_window_height();

	int x, y;
	kinc_mouse_get_position(&x, &y);

	// Guess the new position of X and Y
	int newX = x;
	int newY = y;

	// Correct the position of the X coordinate
	// if the mouse is out the window
	if (x < 0) {
		newX -= x;
	}
	else if (x > width) {
		newX -= x - width;
	}

	// Correct the position of the Y coordinate
	// if the mouse is out the window
	if (y < 0) {
		newY -= y;
	}
	else if (y > height) {
		newY -= y - height;
	}

	// Force the mouse to stay inside the window
	kinc_mouse_set_position(newX, newY);
}

void kinc_internal_mouse_unlock() {
	kinc_mouse_show();
}

bool kinc_mouse_can_lock(void) {
	return true;
}

void kinc_mouse_show() {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	if (mouse_hidden) {
		xlib.XUndefineCursor(x11_ctx.display, window->window);
		mouse_hidden = false;
	}
}

void kinc_mouse_hide() {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
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

void kinc_mouse_set_cursor(int cursor_index) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	if (!mouse_hidden) {
		Cursor cursor;
		switch (cursor_index) {
		case 0: {
			// cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "arrow");
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "left_ptr");
			break;
		}
		case 1: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "hand1");
			break;
		}
		case 2: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "xterm");
			break;
		}
		case 3: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "sb_h_double_arrow");
			break;
		}
		case 4: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "sb_v_double_arrow");
			break;
		}
		case 5: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "top_right_corner");
			break;
		}
		case 6: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "bottom_right_corner");
			break;
		}
		case 7: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "top_left_corner");
			break;
		}
		case 8: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "bottom_left_corner");
			break;
		}
		case 9: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "grab");
			break;
		}
		case 10: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "grabbing");
			break;
		}
		case 11: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "not-allowed");
			break;
		}
		case 12: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "watch");
			break;
		}
		case 13: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "crosshair");
			break;
		}
		default: {
			cursor = xlib.XcursorLibraryLoadCursor(x11_ctx.display, "arrow");
			break;
		}
		}
		xlib.XDefineCursor(x11_ctx.display, window->window, cursor);
	}
}

void kinc_mouse_set_position(int x, int y) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];

	xlib.XWarpPointer(x11_ctx.display, None, window->window, 0, 0, 0, 0, x, y);
	xlib.XFlush(x11_ctx.display); // Flushes the output buffer, therefore updates the cursor's position.
}

void kinc_mouse_get_position(int *x, int *y) {
	struct kinc_x11_window *window = &x11_ctx.windows[0];
	Window inwin;
	Window inchildwin;
	int rootx, rooty;
	unsigned int mask;

	xlib.XQueryPointer(x11_ctx.display, window->window, &inwin, &inchildwin, &rootx, &rooty, x, y, &mask);
}
