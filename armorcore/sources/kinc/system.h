#pragma once

#include <kinc/global.h>
#include <kinc/log.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*! \file system.h
    \brief Provides basic system and application-management functionality which doesn't fit anywhere else.
*/

struct kinc_window_options;
struct kinc_framebuffer_options;

int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame);
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

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#ifdef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <kinc/window.h>
#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#undef KINC_IMPLEMENTATION
#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>
#define KINC_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

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

void kinc_set_application_name(const char *name) {
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
	return kinc_window_width(0);
}

int kinc_height(void) {
	return kinc_window_height(0);
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
	kinc_log(KINC_LOG_LEVEL_WARNING, "Oh no, kinc_copy_to_clipboard is not implemented for this system.");
}
#endif

#endif
