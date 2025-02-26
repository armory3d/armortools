#pragma once

#include <iron_global.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

typedef enum { KINC_LOG_LEVEL_INFO, KINC_LOG_LEVEL_WARNING, KINC_LOG_LEVEL_ERROR } kinc_log_level_t;
void kinc_log(kinc_log_level_t log_level, const char *format, ...);
void kinc_log_args(kinc_log_level_t log_level, const char *format, va_list args);

void kinc_affirm(bool condition);
void kinc_affirm_args(bool condition, const char *format, va_list args);
void kinc_error(void);
void kinc_error_message(const char *format, ...);
void kinc_error_args(const char *format, va_list args);

typedef struct kinc_display_mode {
	int x;
	int y;
	int width;
	int height;
	int pixels_per_inch;
	int frequency;
	int bits_per_pixel;
} kinc_display_mode_t;

void kinc_display_init(void);
int kinc_primary_display(void);
int kinc_count_displays(void);
bool kinc_display_available(int display_index);
const char *kinc_display_name(int display_index);
kinc_display_mode_t kinc_display_current_mode(int display_index);
int kinc_display_count_available_modes(int display_index);
kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index);

typedef struct kinc_framebuffer_options {
	int frequency;
	bool vertical_sync;
	int color_bits;
	int depth_bits;
} kinc_framebuffer_options_t;

typedef enum {
	KINC_WINDOW_MODE_WINDOW,
	KINC_WINDOW_MODE_FULLSCREEN
} kinc_window_mode_t;

#define KINC_WINDOW_FEATURE_RESIZEABLE 1
#define KINC_WINDOW_FEATURE_MINIMIZABLE 2
#define KINC_WINDOW_FEATURE_MAXIMIZABLE 4
#define KINC_WINDOW_FEATURE_BORDERLESS 8
#define KINC_WINDOW_FEATURE_ON_TOP 16

typedef struct kinc_window_options {
	const char *title;
	int x;
	int y;
	int width;
	int height;
	int display_index;
	bool visible;
	int window_features;
	kinc_window_mode_t mode;
} kinc_window_options_t;

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame);
void kinc_window_destroy();
void kinc_window_options_set_defaults(kinc_window_options_t *win);
void kinc_framebuffer_options_set_defaults(kinc_framebuffer_options_t *frame);
void kinc_window_resize(int width, int height);
void kinc_window_move(int x, int y);
void kinc_window_change_mode(kinc_window_mode_t mode);
void kinc_window_change_features(int features);
int kinc_window_x();
int kinc_window_y();
int kinc_window_width();
int kinc_window_height();
int kinc_window_display();
kinc_window_mode_t kinc_window_get_mode();
void kinc_window_show();
void kinc_window_hide();
void kinc_window_set_title(const char *title);
void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data);
void kinc_window_set_close_callback(bool (*callback)(void *data), void *data);

void kinc_internal_call_resize_callback(int width, int height);
bool kinc_internal_call_close_callback();

struct kinc_window_options;
struct kinc_framebuffer_options;

void kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame);
const char *kinc_application_name(void);
void kinc_set_application_name(const char *name);
int kinc_width(void);
int kinc_height(void);
void kinc_load_url(const char *url);
const char *kinc_system_id(void);
const char *kinc_language(void);

typedef uint64_t kinc_ticks_t;

double kinc_frequency(void);
kinc_ticks_t kinc_timestamp(void);
int kinc_cpu_cores(void);
int kinc_hardware_threads(void);
double kinc_time(void);
void kinc_start(void);
void kinc_stop(void);
void kinc_set_keep_screen_on(bool on);

KINC_INLINE void kinc_debug_break(void) {
#ifndef NDEBUG
#if defined(_MSC_VER)
	__debugbreak();
#elif defined(__clang__)
	__builtin_debugtrap();
#else
#if defined(__aarch64__)
	__asm__ volatile(".inst 0xd4200000");
#elif defined(__x86_64__)
	__asm__ volatile("int $0x03");
#else
	kinc_log(KINC_LOG_LEVEL_WARNING, "Oh no, kinc_debug_break is not implemented for the current compiler and CPU.");
#endif
#endif
#endif
}

void kinc_copy_to_clipboard(const char *text);
void kinc_set_update_callback(void (*callback)(void *), void *data);
void kinc_set_foreground_callback(void (*callback)(void *), void *data);
void kinc_set_resume_callback(void (*callback)(void *), void *data);
void kinc_set_pause_callback(void (*callback)(void *), void *data);
void kinc_set_background_callback(void (*callback)(void *), void *data);
void kinc_set_shutdown_callback(void (*callback)(void *), void *data);
void kinc_set_drop_files_callback(void (*callback)(wchar_t *, void *), void *data);
void kinc_set_cut_callback(char *(*callback)(void *), void *data);
void kinc_set_copy_callback(char *(*callback)(void *), void *data);
void kinc_set_paste_callback(void (*callback)(char *, void *), void *data);

bool kinc_internal_frame(void);
const char *kinc_internal_save_path(void);
bool kinc_internal_handle_messages(void);
void kinc_internal_shutdown(void);
void kinc_internal_update_callback(void);
void kinc_internal_foreground_callback(void);
void kinc_internal_resume_callback(void);
void kinc_internal_pause_callback(void);
void kinc_internal_background_callback(void);
void kinc_internal_shutdown_callback(void);
void kinc_internal_drop_files_callback(wchar_t *);
char *kinc_internal_cut_callback(void);
char *kinc_internal_copy_callback(void);
void kinc_internal_paste_callback(char *);
