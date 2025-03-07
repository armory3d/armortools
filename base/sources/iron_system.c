#pragma once

#include "iron_system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef KINC_WINDOWS
#include <backends/windows_mini.h>
#include <backends/windows_system.h>
#endif
#ifdef KINC_ANDROID
#include <android/log.h>
#endif
#include <iron_file.h>
#include <memory.h>
#include <stddef.h>

typedef enum {
	KINC_LOG_LEVEL_INFO,
	KINC_LOG_LEVEL_ERROR
} kinc_log_level_t;

void kinc_log_args(kinc_log_level_t level, const char *format, va_list args) {
#ifdef KINC_ANDROID
	va_list args_android_copy;
	va_copy(args_android_copy, args);
	switch (level) {
	case KINC_LOG_LEVEL_INFO:
		__android_log_vprint(ANDROID_LOG_INFO, "Kinc", format, args_android_copy);
		break;
	case KINC_LOG_LEVEL_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "Kinc", format, args_android_copy);
		break;
	}
	va_end(args_android_copy);
#endif

#ifdef KINC_WINDOWS
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	wcscat(buffer, L"\r\n");
	OutputDebugStringW(buffer);
	DWORD written;
	WriteConsoleW(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)wcslen(buffer), &written, NULL);
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	strcat(buffer, "\n");
	fprintf(level == KINC_LOG_LEVEL_INFO ? stdout : stderr, "%s", buffer);
#endif
}

void kinc_log(const char *format, ...) {
	va_list args;
	va_start(args, format);
	kinc_log_args(KINC_LOG_LEVEL_INFO, format == NULL ? "null" : format, args);
	va_end(args);
}

void kinc_error(const char *format, ...) {
	{
		va_list args;
		va_start(args, format);
		kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KINC_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		kinc_microsoft_format(format, args, buffer);
		MessageBoxW(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif
}

void kinc_window_options_set_defaults(kinc_window_options_t *win) {
	kinc_display_init();
	win->title = NULL;
	win->display_index = kinc_primary_display();
	win->mode = KINC_WINDOW_MODE_WINDOW;
	win->x = -1;
	win->y = -1;
	win->width = 800;
	win->height = 600;
	win->visible = true;
	win->features = KINC_WINDOW_FEATURE_RESIZEABLE | KINC_WINDOW_FEATURE_MINIMIZABLE | KINC_WINDOW_FEATURE_MAXIMIZABLE;

	win->frequency = 60;
	win->vsync = true;
	win->color_bits = 32;
	win->depth_bits = 16;
}

#if !defined(KINC_WASM) && !defined(KINC_ANDROID) && !defined(KINC_WINDOWS)
double kinc_time(void) {
	return kinc_timestamp() / kinc_frequency();
}
#endif

static void (*update_callback)(void *) = NULL;
static void *update_callback_data = NULL;
static void (*foreground_callback)(void *) = NULL;
static void *foreground_callback_data = NULL;
static void (*background_callback)(void *) = NULL;
static void *background_callback_data = NULL;
static void (*pause_callback)(void *) = NULL;
static void *pause_callback_data = NULL;
static void (*resume_callback)(void *) = NULL;
static void *resume_callback_data = NULL;
static void (*shutdown_callback)(void *) = NULL;
static void *shutdown_callback_data = NULL;
static void (*drop_files_callback)(wchar_t *, void *) = NULL;
static void *drop_files_callback_data = NULL;
static char *(*cut_callback)(void *) = NULL;
static void *cut_callback_data = NULL;
static char *(*copy_callback)(void *) = NULL;
static void *copy_callback_data = NULL;
static void (*paste_callback)(char *, void *) = NULL;
static void *paste_callback_data = NULL;

#if defined(KINC_IOS) || defined(KINC_MACOS)
bool withAutoreleasepool(bool (*f)(void));
#endif

void kinc_set_update_callback(void (*callback)(void *), void *data) {
	update_callback = callback;
	update_callback_data = data;
}

void kinc_set_foreground_callback(void (*callback)(void *), void *data) {
	foreground_callback = callback;
	foreground_callback_data = data;
}

void kinc_set_resume_callback(void (*callback)(void *), void *data) {
	resume_callback = callback;
	resume_callback_data = data;
}

void kinc_set_pause_callback(void (*callback)(void *), void *data) {
	pause_callback = callback;
	pause_callback_data = data;
}

void kinc_set_background_callback(void (*callback)(void *), void *data) {
	background_callback = callback;
	background_callback_data = data;
}

void kinc_set_shutdown_callback(void (*callback)(void *), void *data) {
	shutdown_callback = callback;
	shutdown_callback_data = data;
}

void kinc_set_drop_files_callback(void (*callback)(wchar_t *, void *), void *data) {
	drop_files_callback = callback;
	drop_files_callback_data = data;
}

void kinc_set_cut_callback(char *(*callback)(void *), void *data) {
	cut_callback = callback;
	cut_callback_data = data;
}

void kinc_set_copy_callback(char *(*callback)(void *), void *data) {
	copy_callback = callback;
	copy_callback_data = data;
}

void kinc_set_paste_callback(void (*callback)(char *, void *), void *data) {
	paste_callback = callback;
	paste_callback_data = data;
}

void kinc_internal_update_callback(void) {
	if (update_callback != NULL) {
		update_callback(update_callback_data);
	}
}

void kinc_internal_foreground_callback(void) {
	if (foreground_callback != NULL) {
		foreground_callback(foreground_callback_data);
	}
}

void kinc_internal_resume_callback(void) {
	if (resume_callback != NULL) {
		resume_callback(resume_callback_data);
	}
}

void kinc_internal_pause_callback(void) {
	if (pause_callback != NULL) {
		pause_callback(pause_callback_data);
	}
}

void kinc_internal_background_callback(void) {
	if (background_callback != NULL) {
		background_callback(background_callback_data);
	}
}

void kinc_internal_shutdown_callback(void) {
	if (shutdown_callback != NULL) {
		shutdown_callback(shutdown_callback_data);
	}
}

void kinc_internal_drop_files_callback(wchar_t *filePath) {
	if (drop_files_callback != NULL) {
		drop_files_callback(filePath, drop_files_callback_data);
	}
}

char *kinc_internal_cut_callback(void) {
	if (cut_callback != NULL) {
		return cut_callback(cut_callback_data);
	}
	return NULL;
}

char *kinc_internal_copy_callback(void) {
	if (copy_callback != NULL) {
		return copy_callback(copy_callback_data);
	}
	return NULL;
}

void kinc_internal_paste_callback(char *value) {
	if (paste_callback != NULL) {
		paste_callback(value, paste_callback_data);
	}
}

static bool running = false;
static char application_name[1024] = {"Kinc Application"};

const char *kinc_application_name(void) {
	return application_name;
}

void kinc_set_app_name(const char *name) {
	strcpy(application_name, name);
}

void kinc_stop(void) {
	running = false;
}

bool kinc_internal_frame(void) {
	kinc_internal_update_callback();
	kinc_internal_handle_messages();
	return running;
}

void kinc_start(void) {
	running = true;

#if !defined(KINC_WASM)

#if defined(KINC_IOS) || defined(KINC_MACOS)
	while (withAutoreleasepool(kinc_internal_frame)) {
	}
#else
	while (kinc_internal_frame()) {
	}
#endif
	kinc_internal_shutdown();
#endif
}

int kinc_width(void) {
	return kinc_window_width();
}

int kinc_height(void) {
	return kinc_window_height();
}

void kinc_memory_emergency(void) {}

static uint8_t *current_file = NULL;
static size_t current_file_size = 0;

bool kinc_save_file_loaded(void) {
	return true;
}

uint8_t *kinc_get_save_file(void) {
	return current_file;
}

size_t kinc_get_save_file_size(void) {
	return current_file_size;
}

void kinc_load_save_file(const char *filename) {
	free(current_file);
	current_file = NULL;
	current_file_size = 0;

	kinc_file_reader_t reader;
	if (kinc_file_reader_open(&reader, filename, KINC_FILE_TYPE_SAVE)) {
		current_file_size = kinc_file_reader_size(&reader);
		current_file = (uint8_t *)malloc(current_file_size);
		kinc_file_reader_read(&reader, current_file, current_file_size);
		kinc_file_reader_close(&reader);
	}
}

void kinc_save_save_file(const char *filename, uint8_t *data, size_t size) {
	kinc_file_writer_t writer;
	if (kinc_file_writer_open(&writer, filename)) {
		kinc_file_writer_write(&writer, data, (int)size);
		kinc_file_writer_close(&writer);
	}
}

bool kinc_save_is_saving(void) {
	return false;
}

#if !defined(KINC_WINDOWS) && !defined(KINC_LINUX) && !defined(KINC_MACOS)
void kinc_copy_to_clipboard(const char *text) {
	kinc_log("Oh no, kinc_copy_to_clipboard is not implemented for this system.");
}
#endif

static void (*keyboard_key_down_callback)(int /*key_code*/, void * /*data*/) = NULL;
static void *keyboard_key_down_callback_data = NULL;
static void (*keyboard_key_up_callback)(int /*key_code*/, void * /*data*/) = NULL;
static void *keyboard_key_up_callback_data = NULL;
static void (*keyboard_key_press_callback)(unsigned /*character*/, void * /*data*/) = NULL;
static void *keyboard_key_press_callback_data = NULL;

void kinc_keyboard_set_key_down_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data) {
	keyboard_key_down_callback = value;
	keyboard_key_down_callback_data = data;
}

void kinc_keyboard_set_key_up_callback(void (*value)(int /*key_code*/, void * /*data*/), void *data) {
	keyboard_key_up_callback = value;
	keyboard_key_up_callback_data = data;
}

void kinc_keyboard_set_key_press_callback(void (*value)(unsigned /*character*/, void * /*data*/), void *data) {
	keyboard_key_press_callback = value;
	keyboard_key_press_callback_data = data;
}

void kinc_internal_keyboard_trigger_key_down(int key_code) {
	if (keyboard_key_down_callback != NULL) {
		keyboard_key_down_callback(key_code, keyboard_key_down_callback_data);
	}
}

void kinc_internal_keyboard_trigger_key_up(int key_code) {
	if (keyboard_key_up_callback != NULL) {
		keyboard_key_up_callback(key_code, keyboard_key_up_callback_data);
	}
}

void kinc_internal_keyboard_trigger_key_press(unsigned character) {
	if (keyboard_key_press_callback != NULL) {
		keyboard_key_press_callback(character, keyboard_key_press_callback_data);
	}
}

static void (*mouse_press_callback)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/) = NULL;
static void *mouse_press_callback_data = NULL;
static void (*mouse_release_callback)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/) = NULL;
static void *mouse_release_callback_data = NULL;
static void (*mouse_move_callback)(int /*x*/, int /*y*/, int /*movementX*/, int /*movementY*/, void * /*data*/) = NULL;
static void *mouse_move_callback_data = NULL;
static void (*mouse_scroll_callback)(int /*delta*/, void * /*data*/) = NULL;
static void *mouse_scroll_callback_data = NULL;

void kinc_mouse_set_press_callback(void (*value)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data) {
	mouse_press_callback = value;
	mouse_press_callback_data = data;
}

void kinc_mouse_set_release_callback(void (*value)(int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data) {
	mouse_release_callback = value;
	mouse_release_callback_data = data;
}

void kinc_mouse_set_move_callback(void (*value)(int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/, void * /*data*/), void *data) {
	mouse_move_callback = value;
	mouse_move_callback_data = data;
}

void kinc_mouse_set_scroll_callback(void (*value)(int /*delta*/, void * /*data*/), void *data) {
	mouse_scroll_callback = value;
	mouse_scroll_callback_data = data;
}

void kinc_internal_mouse_trigger_release(int button, int x, int y) {
	if (mouse_release_callback != NULL) {
		mouse_release_callback(button, x, y, mouse_release_callback_data);
	}
}

void kinc_internal_mouse_trigger_scroll(int delta) {
	if (mouse_scroll_callback != NULL) {
		mouse_scroll_callback(delta, mouse_scroll_callback_data);
	}
}

void kinc_internal_mouse_window_activated() {
	if (kinc_mouse_is_locked()) {
		kinc_internal_mouse_lock();
	}
}
void kinc_internal_mouse_window_deactivated() {
	if (kinc_mouse_is_locked()) {
		kinc_internal_mouse_unlock();
	}
}

static bool moved = false;
static bool locked = false;
static int preLockWindow = 0;
static int preLockX = 0;
static int preLockY = 0;
static int centerX = 0;
static int centerY = 0;
static int lastX = 0;
static int lastY = 0;

void kinc_internal_mouse_trigger_press(int button, int x, int y) {
	lastX = x;
	lastY = y;
	if (mouse_press_callback != NULL) {
		mouse_press_callback(button, x, y, mouse_press_callback_data);
	}
}

void kinc_internal_mouse_trigger_move(int x, int y) {
	int movementX = 0;
	int movementY = 0;
	if (kinc_mouse_is_locked()) {
		movementX = x - centerX;
		movementY = y - centerY;
		if (movementX != 0 || movementY != 0) {
			kinc_mouse_set_position(centerX, centerY);
			x = centerX;
			y = centerY;
		}
	}
	else if (moved) {
		movementX = x - lastX;
		movementY = y - lastY;
	}
	moved = true;

	lastX = x;
	lastY = y;
	if (mouse_move_callback != NULL && (movementX != 0 || movementY != 0)) {
		mouse_move_callback(x, y, movementX, movementY, mouse_move_callback_data);
	}
}

bool kinc_mouse_is_locked(void) {
	return locked;
}

void kinc_mouse_lock() {
	if (kinc_mouse_is_locked()) {
		return;
	}
	if (!kinc_mouse_can_lock()) {
		return;
	}
	locked = true;
	kinc_internal_mouse_lock();
	kinc_mouse_get_position(&preLockX, &preLockY);
	centerX = kinc_window_width() / 2;
	centerY = kinc_window_height() / 2;
	kinc_mouse_set_position(centerX, centerY);
}

void kinc_mouse_unlock(void) {
	if (!kinc_mouse_is_locked()) {
		return;
	}
	if (!kinc_mouse_can_lock()) {
		return;
	}
	moved = false;
	locked = false;
	kinc_internal_mouse_unlock();
	kinc_mouse_set_position(preLockX, preLockY);
}

static void (*pen_press_callback)(int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*pen_move_callback)(int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*pen_release_callback)(int /*x*/, int /*y*/, float /*pressure*/) = NULL;

static void (*eraser_press_callback)(int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*eraser_move_callback)(int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*eraser_release_callback)(int /*x*/, int /*y*/, float /*pressure*/) = NULL;

void kinc_pen_set_press_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/)) {
	pen_press_callback = value;
}

void kinc_pen_set_move_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/)) {
	pen_move_callback = value;
}

void kinc_pen_set_release_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/)) {
	pen_release_callback = value;
}

void kinc_eraser_set_press_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/)) {
	eraser_press_callback = value;
}

void kinc_eraser_set_move_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/)) {
	eraser_move_callback = value;
}

void kinc_eraser_set_release_callback(void (*value)(int /*x*/, int /*y*/, float /*pressure*/)) {
	eraser_release_callback = value;
}

void kinc_internal_pen_trigger_press(int x, int y, float pressure) {
	if (pen_press_callback != NULL) {
		pen_press_callback(x, y, pressure);
	}
}

void kinc_internal_pen_trigger_move(int x, int y, float pressure) {
	if (pen_move_callback != NULL) {
		pen_move_callback(x, y, pressure);
	}
}

void kinc_internal_pen_trigger_release(int x, int y, float pressure) {
	if (pen_release_callback != NULL) {
		pen_release_callback(x, y, pressure);
	}
}

void kinc_internal_eraser_trigger_press(int x, int y, float pressure) {
	if (eraser_press_callback != NULL) {
		eraser_press_callback(x, y, pressure);
	}
}

void kinc_internal_eraser_trigger_move(int x, int y, float pressure) {
	if (eraser_move_callback != NULL) {
		eraser_move_callback(x, y, pressure);
	}
}

void kinc_internal_eraser_trigger_release(int x, int y, float pressure) {
	if (eraser_release_callback != NULL) {
		eraser_release_callback(x, y, pressure);
	}
}

static void (*surface_touch_start_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
static void (*surface_move_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
static void (*surface_touch_end_callback)(int /*index*/, int /*x*/, int /*y*/) = NULL;

void kinc_surface_set_touch_start_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/)) {
	surface_touch_start_callback = value;
}

void kinc_surface_set_move_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/)) {
	surface_move_callback = value;
}

void kinc_surface_set_touch_end_callback(void (*value)(int /*index*/, int /*x*/, int /*y*/)) {
	surface_touch_end_callback = value;
}

void kinc_internal_surface_trigger_touch_start(int index, int x, int y) {
	if (surface_touch_start_callback != NULL) {
		surface_touch_start_callback(index, x, y);
	}
}

void kinc_internal_surface_trigger_move(int index, int x, int y) {
	if (surface_move_callback != NULL) {
		surface_move_callback(index, x, y);
	}
}

void kinc_internal_surface_trigger_touch_end(int index, int x, int y) {
	if (surface_touch_end_callback != NULL) {
		surface_touch_end_callback(index, x, y);
	}
}

static void (*gamepad_connect_callback)(int /*gamepad*/, void * /*userdata*/) = NULL;
static void *gamepad_connect_callback_userdata = NULL;
static void (*gamepad_disconnect_callback)(int /*gamepad*/, void * /*userdata*/) = NULL;
static void *gamepad_disconnect_callback_userdata = NULL;
static void (*gamepad_axis_callback)(int /*gamepad*/, int /*axis*/, float /*value*/, void * /*userdata*/) = NULL;
static void *gamepad_axis_callback_userdata = NULL;
static void (*gamepad_button_callback)(int /*gamepad*/, int /*button*/, float /*value*/, void * /*userdata*/) = NULL;
static void *gamepad_button_callback_userdata = NULL;

void kinc_gamepad_set_connect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata) {
	gamepad_connect_callback = value;
	gamepad_connect_callback_userdata = userdata;
}

void kinc_gamepad_set_disconnect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata) {
	gamepad_disconnect_callback = value;
	gamepad_disconnect_callback_userdata = userdata;
}

void kinc_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/, void * /*userdata*/), void *userdata) {
	gamepad_axis_callback = value;
	gamepad_axis_callback_userdata = userdata;
}

void kinc_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/, void * /*userdata*/), void *userdata) {
	gamepad_button_callback = value;
	gamepad_button_callback_userdata = userdata;
}

void kinc_internal_gamepad_trigger_connect(int gamepad) {
	if (gamepad_connect_callback != NULL) {
		gamepad_connect_callback(gamepad, gamepad_connect_callback_userdata);
	}
}

void kinc_internal_gamepad_trigger_disconnect(int gamepad) {
	if (gamepad_disconnect_callback != NULL) {
		gamepad_disconnect_callback(gamepad, gamepad_disconnect_callback_userdata);
	}
}

void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value) {
	if (gamepad_axis_callback != NULL) {
		gamepad_axis_callback(gamepad, axis, value, gamepad_axis_callback_userdata);
	}
}

void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value) {
	if (gamepad_button_callback != NULL) {
		gamepad_button_callback(gamepad, button, value, gamepad_button_callback_userdata);
	}
}
