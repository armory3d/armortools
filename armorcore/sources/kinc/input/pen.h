#pragma once

#include <kinc/global.h>

/*! \file pen.h
    \brief Provides pen-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Sets the pen-press-callback which is called when the pen is touching the drawing-surface.
/// </summary>
/// <param name="value">The callback</param>
void kinc_pen_set_press_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the pen-move-callback which is called when the pen is moved.
/// </summary>
/// <param name="value">The callback</param>
void kinc_pen_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the pen-release-callback which is called when the pen is moved away from the drawing-surface.
/// </summary>
/// <param name="value">The callback</param>
void kinc_pen_set_release_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the eraser-press-callback which is called when the pen is touching the drawing-surface in eraser-mode.
/// </summary>
/// <param name="value">The callback</param>
void kinc_eraser_set_press_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the eraser-move-callback which is called when the pen is moved while in eraser-mode.
/// </summary>
/// <param name="value">The callback</param>
void kinc_eraser_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

/// <summary>
/// Sets the eraser-release-callback which is called when the pen is moved away from the drawing-surface when in eraser-mode.
/// </summary>
/// <param name="value">The callback</param>
void kinc_eraser_set_release_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/));

void kinc_internal_pen_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_pen_trigger_release(int window, int x, int y, float pressure);

void kinc_internal_eraser_trigger_move(int window, int x, int y, float pressure);
void kinc_internal_eraser_trigger_press(int window, int x, int y, float pressure);
void kinc_internal_eraser_trigger_release(int window, int x, int y, float pressure);

#ifdef KINC_IMPLEMENTATION_INPUT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <memory.h>
#include <stddef.h>

static void (*pen_press_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*pen_move_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*pen_release_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;

static void (*eraser_press_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*eraser_move_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
static void (*eraser_release_callback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;

void kinc_pen_set_press_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/)) {
	pen_press_callback = value;
}

void kinc_pen_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/)) {
	pen_move_callback = value;
}

void kinc_pen_set_release_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/)) {
	pen_release_callback = value;
}

void kinc_eraser_set_press_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/)) {
	eraser_press_callback = value;
}

void kinc_eraser_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/)) {
	eraser_move_callback = value;
}

void kinc_eraser_set_release_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/)) {
	eraser_release_callback = value;
}

void kinc_internal_pen_trigger_press(int window, int x, int y, float pressure) {
	if (pen_press_callback != NULL) {
		pen_press_callback(window, x, y, pressure);
	}
}

void kinc_internal_pen_trigger_move(int window, int x, int y, float pressure) {
	if (pen_move_callback != NULL) {
		pen_move_callback(window, x, y, pressure);
	}
}

void kinc_internal_pen_trigger_release(int window, int x, int y, float pressure) {
	if (pen_release_callback != NULL) {
		pen_release_callback(window, x, y, pressure);
	}
}

void kinc_internal_eraser_trigger_press(int window, int x, int y, float pressure) {
	if (eraser_press_callback != NULL) {
		eraser_press_callback(window, x, y, pressure);
	}
}

void kinc_internal_eraser_trigger_move(int window, int x, int y, float pressure) {
	if (eraser_move_callback != NULL) {
		eraser_move_callback(window, x, y, pressure);
	}
}

void kinc_internal_eraser_trigger_release(int window, int x, int y, float pressure) {
	if (eraser_release_callback != NULL) {
		eraser_release_callback(window, x, y, pressure);
	}
}

#endif

#ifdef __cplusplus
}
#endif
