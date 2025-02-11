#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>

int kinc_window_x(int window) {
	return 0;
}

int kinc_window_y(int window) {
	return 0;
}

void kinc_window_resize(int window, int width, int height) {}

void kinc_window_move(int window, int x, int y) {}

void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame);

void kinc_window_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {
	kinc_internal_change_framebuffer(0, frame);
}

void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {}

void kinc_window_change_features(int window, int features) {}

void kinc_window_change_mode(int window, kinc_window_mode_t mode) {}

void kinc_window_destroy(int window) {}

void kinc_window_show(int window) {}

void kinc_window_hide(int window) {}

void kinc_window_set_title(int window, const char *title) {}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	windowCounter += 1;
	return 0;
}

void kinc_window_set_resize_callback(int window, void (*callback)(int x, int y, void *data), void *data) {
	assert(window < windowCounter);
	windows[window].resizeCallback = callback;
	windows[window].resizeCallbackData = data;
}

kinc_window_mode_t kinc_window_get_mode(int window) {
	return KINC_WINDOW_MODE_WINDOW;
}

int kinc_window_display(int window) {
	return 0;
}
