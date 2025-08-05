
#include <iron_system.h>
#include <iron_audio.h>
#include <iron_gpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void iron_display_init(void) {}

int iron_primary_display(void) {
	return 0;
}

int iron_count_displays(void) {
	return 1;
}

iron_display_mode_t iron_display_current_mode(int display_index) {
	iron_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = 800;
	mode.height = 600;
	mode.pixels_per_inch = 96;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	return mode;
}

void iron_internal_mouse_lock() {}

void iron_internal_mouse_unlock(void) {}

bool iron_mouse_can_lock(void) {
	return false;
}

void iron_mouse_show() {}

void iron_mouse_hide() {}

void iron_mouse_set_position(int x, int y) {}

void iron_mouse_get_position(int *x, int *y) {}

__attribute__((import_module("imports"), import_name("js_time"))) int js_time();

extern int iron_internal_window_width;
extern int iron_internal_window_height;

void iron_init(iron_window_options_t *win) {
	iron_internal_window_width = win->width;
	iron_internal_window_height = win->height;
	gpu_init(win->depth_bits, true);
}

bool iron_internal_handle_messages() {
	return true;
}

void iron_set_keep_screen_on(bool on) {}

double iron_frequency(void) {
	return 1000.0;
}

uint64_t iron_timestamp(void) {
	return (uint64_t)(js_time());
}

double iron_time(void) {
	return js_time() / 1000.0;
}

int iron_hardware_threads(void) {
	return 4;
}

void iron_internal_shutdown(void) {}

extern int kickstart(int argc, char **argv);

__attribute__((export_name("_start"))) void _start(void) {
	kickstart(0, NULL);
}

__attribute__((export_name("_update"))) void _update(void) {
	iron_internal_update_callback();
	iron_a2_update();
}

__attribute__((export_name("_mousedown"))) void _mousedown(int button, int x, int y) {
	iron_internal_mouse_trigger_press(0, button, x, y);
}

__attribute__((export_name("_mouseup"))) void _mouseup(int button, int x, int y) {
	iron_internal_mouse_trigger_release(0, button, x, y);
}

__attribute__((export_name("_mousemove"))) void _mousemove(int x, int y) {
	iron_internal_mouse_trigger_move(0, x, y);
}

__attribute__((export_name("_wheel"))) void _wheel(int delta) {
	iron_internal_mouse_trigger_scroll(0, delta);
}

__attribute__((export_name("_keydown"))) void _keydown(int key) {
	iron_internal_keyboard_trigger_key_down(key);
}

__attribute__((export_name("_keyup"))) void _keyup(int key) {
	iron_internal_keyboard_trigger_key_up(key);
}

int iron_internal_window_width = 0;
int iron_internal_window_height = 0;
iron_window_mode_t iron_internal_window_mode = IRON_WINDOW_MODE_WINDOW;

int iron_window_x() {
	return 0;
}

int iron_window_y() {
	return 0;
}

int iron_window_width() {
	return iron_internal_window_width;
}

int iron_window_height() {
	return iron_internal_window_height;
}

void iron_window_resize(int width, int height) {}

void iron_window_move(int x, int y) {}

// In HTML5 fullscreen is activable only from user input.
void iron_window_change_mode(iron_window_mode_t mode) {
	if (mode == IRON_WINDOW_MODE_FULLSCREEN) {
		if (iron_internal_window_mode == IRON_WINDOW_MODE_FULLSCREEN) {
			iron_internal_window_mode = mode;
			return;
		}
		// TODO: call js Fullscreen API
		iron_internal_window_mode = mode;
	}
	else {
		if (mode == iron_internal_window_mode) {
			return;
		}
		// TODO: call js Fullscreen API
		iron_internal_window_mode = mode;
	}
}

void iron_window_destroy() {}

void iron_window_show() {}

void iron_window_hide() {}

// TODO: change browser title.
void iron_window_set_title(const char *title) {}

void iron_window_create(iron_window_options_t *win) {}

void iron_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {}

void iron_window_set_close_callback(bool (*callback)(void *), void *data) {}

iron_window_mode_t iron_window_get_mode() {
	return iron_internal_window_mode;
}

