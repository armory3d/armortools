#include <kinc/audio2/audio.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((import_module("imports"), import_name("js_time"))) int js_time();

extern int kinc_internal_window_width;
extern int kinc_internal_window_height;

int kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
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

	kinc_g4_internal_init();
	kinc_g4_internal_init_window(0, frame->depth_bits, true);

	return 0;
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
