#pragma once

#include <stdbool.h>
#include <kinc/global.h>

/*! \file window.h
    \brief Provides functionality for creating and handling windows.
*/

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

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#ifdef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <kinc/display.h>
#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#include <stdlib.h>

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
	win->window_features = KINC_WINDOW_FEATURE_RESIZEABLE | KINC_WINDOW_FEATURE_MINIMIZABLE | KINC_WINDOW_FEATURE_MAXIMIZABLE;
}

void kinc_framebuffer_options_set_defaults(kinc_framebuffer_options_t *frame) {
	frame->frequency = 60;
	frame->vertical_sync = true;
	frame->color_bits = 32;
	frame->depth_bits = 16;
}

#endif
