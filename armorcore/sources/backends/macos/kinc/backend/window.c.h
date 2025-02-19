#include <kinc/display.h>
#include <kinc/graphics5/g5.h>
#include <kinc/window.h>

int kinc_window_x() {
	return 0;
}

int kinc_window_y() {
	return 0;
}

void kinc_window_resize(int width, int height) {}

void kinc_window_move(int x, int y) {}

void kinc_window_change_features(int features) {}

void kinc_window_change_mode(kinc_window_mode_t mode) {}

void kinc_window_destroy() {}

void kinc_window_show() {}

void kinc_window_hide() {}

void kinc_window_set_title(const char *title) {}

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	return 0;
}

void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {
	windows[0].resizeCallback = callback;
	windows[0].resizeCallbackData = data;
}

kinc_window_mode_t kinc_window_get_mode() {
	return KINC_WINDOW_MODE_WINDOW;
}

int kinc_window_display() {
	return 0;
}
