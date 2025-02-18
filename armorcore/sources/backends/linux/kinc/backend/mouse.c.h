#include "funcs.h"
#include <kinc/mouse.h>

void kinc_internal_mouse_lock() {
	procs.mouse_lock();
}

void kinc_internal_mouse_unlock() {
	procs.mouse_unlock();
}

bool kinc_mouse_can_lock(void) {
	return true;
}

bool _mouseHidden = false;

void kinc_mouse_show() {
	procs.mouse_show();
}

void kinc_mouse_hide() {
	procs.mouse_hide();
}

void kinc_mouse_set_cursor(int cursor_index) {
	procs.mouse_set_cursor(cursor_index);
}

void kinc_mouse_set_position(int x, int y) {
	procs.mouse_set_position(x, y);
}

void kinc_mouse_get_position(int *x, int *y) {
	procs.mouse_get_position(x, y);
}
