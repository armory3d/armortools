#include <kinc/display.h>
#include <kinc/g5.h>
#include <kinc/window.h>

static void (*resizeCallback)(int x, int y, void *data) = NULL;
static void *resizeCallbackData = NULL;

int kinc_window_x() {
	return 0;
}

int kinc_window_y() {
	return 0;
}

int kinc_android_width();

int kinc_window_width() {
	return kinc_android_width();
}

int kinc_android_height();

int kinc_window_height() {
	return kinc_android_height();
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

}

void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {
	resizeCallback = callback;
	resizeCallbackData = data;
}

void kinc_internal_call_resize_callback(int width, int height) {
	if (resizeCallback != NULL) {
		resizeCallback(width, height, resizeCallbackData);
	}
}

void kinc_window_set_close_callback(bool (*callback)(void *), void *data) {}

kinc_window_mode_t kinc_window_get_mode() {
	return KINC_WINDOW_MODE_FULLSCREEN;
}

int kinc_window_display() {
	return 0;
}
