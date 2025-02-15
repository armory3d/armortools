#include <kinc/display.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/window.h>
#include <string.h>

int kinc_internal_window_width = 0;
int kinc_internal_window_height = 0;
kinc_window_mode_t kinc_internal_window_mode = KINC_WINDOW_MODE_WINDOW;

int kinc_window_x() {
	return 0;
}

int kinc_window_y() {
	return 0;
}

int kinc_window_width() {
	return kinc_internal_window_width;
}

int kinc_window_height() {
	return kinc_internal_window_height;
}

void kinc_window_resize(int width, int height) {}

void kinc_window_move(int x, int y) {}

void kinc_window_change_features(int features) {}

// In HTML5 fullscreen is activable only from user input.
void kinc_window_change_mode(kinc_window_mode_t mode) {
	if (mode == KINC_WINDOW_MODE_FULLSCREEN) {
		if (kinc_internal_window_mode == KINC_WINDOW_MODE_FULLSCREEN) {
			kinc_internal_window_mode = mode;
			return;
		}
		// TODO: call js Fullscreen API
		kinc_internal_window_mode = mode;
	}
	else {
		if (mode == kinc_internal_window_mode) {
			return;
		}
		// TODO: call js Fullscreen API
		kinc_internal_window_mode = mode;
	}
}

void kinc_window_destroy() {}

void kinc_window_show() {}

void kinc_window_hide() {}

// TODO: change browser title.
void kinc_window_set_title(const char *title) {}

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {

}

void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {}

void kinc_window_set_close_callback(bool (*callback)(void *), void *data) {}

kinc_window_mode_t kinc_window_get_mode() {
	return kinc_internal_window_mode;
}
