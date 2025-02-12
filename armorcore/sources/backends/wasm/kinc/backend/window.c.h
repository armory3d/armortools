#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>
#include <string.h>

int kinc_internal_window_width = 0;
int kinc_internal_window_height = 0;
kinc_window_mode_t kinc_internal_window_mode = KINC_WINDOW_MODE_WINDOW;

int kinc_count_windows(void) {
	return 1;
}

int kinc_window_x(int window_index) {
	return 0;
}

int kinc_window_y(int window_index) {
	return 0;
}

int kinc_window_width(int window_index) {
	return kinc_internal_window_width;
}

int kinc_window_height(int window_index) {
	return kinc_internal_window_height;
}

void kinc_window_resize(int window_index, int width, int height) {}

void kinc_window_move(int window_index, int x, int y) {}

void kinc_window_change_framebuffer(int window_index, kinc_framebuffer_options_t *frame) {
	//**kinc_g4_changeFramebuffer(0, frame);
}

void kinc_window_change_features(int window_index, int features) {}

// In HTML5 fullscreen is activable only from user input.
void kinc_window_change_mode(int window_index, kinc_window_mode_t mode) {
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

void kinc_window_destroy(int window_index) {}

void kinc_window_show(int window_index) {}

void kinc_window_hide(int window_index) {}

// TODO: change browser title.
void kinc_window_set_title(int window_index, const char *title) {}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	return 0;
}

void kinc_window_set_resize_callback(int window_index, void (*callback)(int x, int y, void *data), void *data) {}

void kinc_window_set_close_callback(int window, bool (*callback)(void *), void *data) {}

kinc_window_mode_t kinc_window_get_mode(int window_index) {
	return kinc_internal_window_mode;
}
