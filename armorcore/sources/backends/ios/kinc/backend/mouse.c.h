#include <kinc/input/mouse.h>

void kinc_internal_mouse_lock(int window) {}

void kinc_internal_mouse_unlock(void) {}

bool kinc_mouse_can_lock(void) {
	return false;
}

void kinc_mouse_show(void) {}

void kinc_mouse_hide(void) {}

void kinc_mouse_set_position(int window, int x, int y) {}

void kinc_mouse_get_position(int window, int *x, int *y) {}

void kinc_mouse_set_cursor(int cursor_index) {}
