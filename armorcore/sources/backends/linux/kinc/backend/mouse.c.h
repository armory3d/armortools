#include "funcs.h"
#include <kinc/input/mouse.h>

void kinc_internal_mouse_lock(int window) {
	procs.mouse_lock(window);
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

void kinc_mouse_set_cursor(int cursorIndex) {
	procs.mouse_set_cursor(cursorIndex);
}

void kinc_mouse_set_position(int window, int x, int y) {
	procs.mouse_set_position(window, x, y);
}

void kinc_mouse_get_position(int window, int *x, int *y) {
	procs.mouse_get_position(window, x, y);
}
