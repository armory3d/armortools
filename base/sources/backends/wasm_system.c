
#include <iron_system.h>
#include <iron_audio.h>
#include <iron_gpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void kinc_display_init(void) {}

int kinc_primary_display(void) {
	return 0;
}

int kinc_count_displays(void) {
	return 1;
}

bool kinc_display_available(int display_index) {
	return false;
}

const char *kinc_display_name(int display_index) {
	return "Browser";
}

kinc_display_mode_t kinc_display_current_mode(int display_index) {
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = 800;
	mode.height = 600;
	mode.pixels_per_inch = 96;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	return mode;
}

int kinc_display_count_available_modes(int display_index) {
	return 1;
}

kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index) {
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = 800;
	mode.height = 600;
	mode.pixels_per_inch = 96;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	return mode;
}

void kinc_internal_mouse_lock() {}

void kinc_internal_mouse_unlock(void) {}

bool kinc_mouse_can_lock(void) {
	return false;
}

void kinc_mouse_show() {}

void kinc_mouse_hide() {}

void kinc_mouse_set_position(int x, int y) {}

void kinc_mouse_get_position(int *x, int *y) {}

__attribute__((import_module("imports"), import_name("js_time"))) int js_time();

extern int kinc_internal_window_width;
extern int kinc_internal_window_height;

void kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}
	win->width = width;
	win->height = height;

	kinc_internal_window_width = width;
	kinc_internal_window_height = height;

	kinc_g5_internal_init();
	kinc_g4_internal_init_window(frame->depth_bits, true);
}

bool kinc_internal_handle_messages() {
	return true;
}

void kinc_set_keep_screen_on(bool on) {}

double kinc_frequency(void) {
	return 1000.0;
}

kinc_ticks_t kinc_timestamp(void) {
	return (kinc_ticks_t)(js_time());
}

double kinc_time(void) {
	return js_time() / 1000.0;
}

int kinc_cpu_cores(void) {
	return 4;
}

int kinc_hardware_threads(void) {
	return 4;
}

void kinc_internal_shutdown(void) {}

extern int kickstart(int argc, char **argv);

__attribute__((export_name("_start"))) void _start(void) {
	kickstart(0, NULL);
}

__attribute__((export_name("_update"))) void _update(void) {
	kinc_internal_update_callback();
	kinc_a2_update();
}

__attribute__((export_name("_mousedown"))) void _mousedown(int button, int x, int y) {
	kinc_internal_mouse_trigger_press(0, button, x, y);
}

__attribute__((export_name("_mouseup"))) void _mouseup(int button, int x, int y) {
	kinc_internal_mouse_trigger_release(0, button, x, y);
}

__attribute__((export_name("_mousemove"))) void _mousemove(int x, int y) {
	kinc_internal_mouse_trigger_move(0, x, y);
}

__attribute__((export_name("_wheel"))) void _wheel(int delta) {
	kinc_internal_mouse_trigger_scroll(0, delta);
}

__attribute__((export_name("_keydown"))) void _keydown(int key) {
	kinc_internal_keyboard_trigger_key_down(key);
}

__attribute__((export_name("_keyup"))) void _keyup(int key) {
	kinc_internal_keyboard_trigger_key_up(key);
}

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

