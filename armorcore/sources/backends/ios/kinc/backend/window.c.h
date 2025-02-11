#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>

static void (*resizeCallback)(int x, int y, void *data) = NULL;
static void *resizeCallbackData = NULL;

int kinc_window_x(int window) {
	return 0;
}

int kinc_window_y(int window) {
	return 0;
}

int kinc_count_windows(void) {
	return 1;
}

void kinc_window_resize(int window, int width, int height) {}

void kinc_window_move(int window, int x, int y) {}

void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame);

void kinc_window_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {
	kinc_internal_change_framebuffer(0, frame);
}

#ifdef KINC_METAL
void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {}
#endif

void kinc_window_change_features(int window, int features) {}

void kinc_window_change_mode(int window, kinc_window_mode_t mode) {}

void kinc_window_destroy(int window) {}

void kinc_window_show(int window) {}

void kinc_window_hide(int window) {}

void kinc_window_set_title(int window, const char *title) {}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	return 0;
}

void kinc_window_set_resize_callback(int window, void (*callback)(int x, int y, void *data), void *data) {
	resizeCallback = callback;
	resizeCallbackData = data;
}

void kinc_internal_call_resize_callback(int window, int width, int height) {
	if (resizeCallback != NULL) {
		resizeCallback(width, height, resizeCallbackData);
	}
}

void kinc_window_set_close_callback(int window, bool (*callback)(void *), void *data) {}

kinc_window_mode_t kinc_window_get_mode(int window) {
	return KINC_WINDOW_MODE_FULLSCREEN;
}

int kinc_window_display(int window) {
	return 0;
}
