#include <kinc/backend/windows.h>
#include <iron_system.h>
#include <iron_input.h>

void kinc_internal_mouse_lock() {
	kinc_mouse_hide();
	HWND handle = kinc_windows_window_handle();
	SetCapture(handle);
	RECT rect;
	GetWindowRect(handle, &rect);
	ClipCursor(&rect);
}

void kinc_internal_mouse_unlock(void) {
	kinc_mouse_show();
	ReleaseCapture();
	ClipCursor(NULL);
}

bool kinc_mouse_can_lock(void) {
	return true;
}

void kinc_mouse_show() {
	// Work around the internal counter of ShowCursor
	while (ShowCursor(true) < 0) {
	}
}

void kinc_mouse_hide() {
	// Work around the internal counter of ShowCursor
	while (ShowCursor(false) >= 0) {
	}
}

void kinc_mouse_set_position(int x, int y) {
	POINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(kinc_windows_window_handle(), &point);
	SetCursorPos(point.x, point.y);
}

void kinc_mouse_get_position(int *x, int *y) {
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(kinc_windows_window_handle(), &point);
	*x = point.x;
	*y = point.y;
}
