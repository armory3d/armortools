#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file mouse.h
    \brief Provides mouse-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_MOUSE_LEFT 0
#define KINC_MOUSE_RIGHT 1
#define KINC_MOUSE_MIDDLE 2
// eg backward sidebutton
#define KINC_MOUSE_EXTRA1 3
// eg forward sidebutton
#define KINC_MOUSE_EXTRA2 4

/// <summary>
/// Sets the mouse-press-callback which is called when a mouse-button is pressed.
/// The button callback argument will usually contain one of the KINC_MOUSE_* values,
/// but can contain higher values on certain platforms when mice with a lot of buttons are used.
/// </summary>
/// <param name="value">The callback</param>
void kinc_mouse_set_press_callback(void (*value)(int /*window*/, int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data);

/// <summary>
/// Sets the mouse-release-callback which is called when a mouse-button is released.
/// The button callback argument will usually contain one of the KINC_MOUSE_* values,
/// but can contain higher values on certain platforms when mice with a lot of buttons are used.
/// </summary>
/// <param name="value">The callback</param>
void kinc_mouse_set_release_callback(void (*value)(int /*window*/, int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data);

/// <summary>
/// Sets the mouse-move-callback which is called when the mouse is moved.
/// </summary>
/// <param name="value">The callback</param>
void kinc_mouse_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/, void * /*data*/),
                                            void *data);

/// <summary>
/// Sets the mouse-scroll-callback which is called when the mouse wheel is spinning.
/// </summary>
/// <param name="value">The callback</param>
void kinc_mouse_set_scroll_callback(void (*value)(int /*window*/, int /*delta*/, void * /*data*/), void *data);

/// <summary>
/// Sets the mouse-enter-window-callback which is called when the mouse-cursor enters the application-window.
/// </summary>
/// <param name="value">The callback</param>
void kinc_mouse_set_enter_window_callback(void (*value)(int /*window*/, void * /*data*/), void *data);

/// <summary>
/// Sets the mouse-leave-window-callback which is called when the mouse-cursor leaves the application-window.
/// </summary>
/// <param name="value">The callback</param>
void kinc_mouse_set_leave_window_callback(void (*value)(int /*window*/, void * /*data*/), void *data);

/// <summary>
/// Figures out whether mouse-locking is supported.
/// </summary>
/// <returns>Whether mouse-locking is supported</returns>
bool kinc_mouse_can_lock(void);

/// <summary>
/// Figures out whether the mouse is currently locked.
/// </summary>
/// <returns>Whether the mouse is currently locked</returns>
bool kinc_mouse_is_locked(void);

/// <summary>
/// Locks the mouse to a window.
/// </summary>
/// <param name="window">The window to lock the mouse to</param>
void kinc_mouse_lock(int window);

/// <summary>
/// Unlocks the mouse.
/// </summary>
/// <param name=""></param>
/// <returns></returns>
void kinc_mouse_unlock(void);

/// <summary>
/// Change the cursor-image to something that's semi-randomly based on the provided int.
/// </summary>
/// <param name="cursor">Defines what the cursor is changed to - somehow</param>
void kinc_mouse_set_cursor(int cursor);

/// <summary>
/// Shows the mouse-cursor.
/// </summary>
void kinc_mouse_show(void);

/// <summary>
/// Hides the mouse-cursor.
/// </summary>
void kinc_mouse_hide(void);

/// <summary>
/// Manually sets the mouse-cursor-position.
/// </summary>
/// <param name="window">The window to place the cursor in</param>
/// <param name="x">The x-position inside the window to place the cursor at</param>
/// <param name="y">The y-position inside the window to place the cursor at</param>
void kinc_mouse_set_position(int window, int x, int y);

/// <summary>
/// Gets the current mouse-position relative to a window.
/// </summary>
/// <param name="window">The window to base the returned position on</param>
/// <param name="x">A pointer where the cursor's x-position is put into</param>
/// <param name="y">A pointer where the cursor's y-position is put into</param>
void kinc_mouse_get_position(int window, int *x, int *y);

void kinc_internal_mouse_trigger_press(int window, int button, int x, int y);
void kinc_internal_mouse_trigger_release(int window, int button, int x, int y);
void kinc_internal_mouse_trigger_move(int window, int x, int y);
void kinc_internal_mouse_trigger_scroll(int window, int delta);
void kinc_internal_mouse_trigger_enter_window(int window);
void kinc_internal_mouse_trigger_leave_window(int window);
void kinc_internal_mouse_lock(int window);
void kinc_internal_mouse_unlock(void);
void kinc_internal_mouse_window_activated(int window);
void kinc_internal_mouse_window_deactivated(int window);

#ifdef KINC_IMPLEMENTATION_INPUT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#undef KINC_IMPLEMENTATION
#include <kinc/window.h>
#define KINC_IMPLEMENTATION

#include <memory.h>
#include <stddef.h>

static void (*mouse_press_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/, void * /*data*/) = NULL;
static void *mouse_press_callback_data = NULL;
static void (*mouse_release_callback)(int /*window*/, int /*button*/, int /*x*/, int /*y*/, void * /*data*/) = NULL;
static void *mouse_release_callback_data = NULL;
static void (*mouse_move_callback)(int /*window*/, int /*x*/, int /*y*/, int /*movementX*/, int /*movementY*/, void * /*data*/) = NULL;
static void *mouse_move_callback_data = NULL;
static void (*mouse_scroll_callback)(int /*window*/, int /*delta*/, void * /*data*/) = NULL;
static void *mouse_scroll_callback_data = NULL;
static void (*mouse_enter_window_callback)(int /*window*/, void * /*data*/) = NULL;
static void *mouse_enter_window_callback_data = NULL;
static void (*mouse_leave_window_callback)(int /*window*/, void * /*data*/) = NULL;
static void *mouse_leave_window_callback_data = NULL;

void kinc_mouse_set_press_callback(void (*value)(int /*window*/, int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data) {
	mouse_press_callback = value;
	mouse_press_callback_data = data;
}

void kinc_mouse_set_release_callback(void (*value)(int /*window*/, int /*button*/, int /*x*/, int /*y*/, void * /*data*/), void *data) {
	mouse_release_callback = value;
	mouse_release_callback_data = data;
}

void kinc_mouse_set_move_callback(void (*value)(int /*window*/, int /*x*/, int /*y*/, int /*movement_x*/, int /*movement_y*/, void * /*data*/), void *data) {
	mouse_move_callback = value;
	mouse_move_callback_data = data;
}

void kinc_mouse_set_scroll_callback(void (*value)(int /*window*/, int /*delta*/, void * /*data*/), void *data) {
	mouse_scroll_callback = value;
	mouse_scroll_callback_data = data;
}

void kinc_mouse_set_enter_window_callback(void (*value)(int /*window*/, void * /*data*/), void *data) {
	mouse_enter_window_callback = value;
	mouse_enter_window_callback_data = data;
}

void kinc_mouse_set_leave_window_callback(void (*value)(int /*window*/, void * /*data*/), void *data) {
	mouse_leave_window_callback = value;
	mouse_leave_window_callback_data = data;
}

void kinc_internal_mouse_trigger_release(int window, int button, int x, int y) {
	if (mouse_release_callback != NULL) {
		mouse_release_callback(window, button, x, y, mouse_release_callback_data);
	}
}

void kinc_internal_mouse_trigger_scroll(int window, int delta) {
	if (mouse_scroll_callback != NULL) {
		mouse_scroll_callback(window, delta, mouse_scroll_callback_data);
	}
}

void kinc_internal_mouse_trigger_enter_window(int window) {
	if (mouse_enter_window_callback != NULL) {
		mouse_enter_window_callback(window, mouse_enter_window_callback_data);
	}
}

void kinc_internal_mouse_trigger_leave_window(int window) {
	if (mouse_leave_window_callback != NULL) {
		mouse_leave_window_callback(window, mouse_leave_window_callback_data);
	}
}

void kinc_internal_mouse_window_activated(int window) {
	if (kinc_mouse_is_locked()) {
		kinc_internal_mouse_lock(window);
	}
}
void kinc_internal_mouse_window_deactivated(int window) {
	if (kinc_mouse_is_locked()) {
		kinc_internal_mouse_unlock();
	}
}

// TODO: handle state per window
static bool moved = false;
static bool locked = false;
static int preLockWindow = 0;
static int preLockX = 0;
static int preLockY = 0;
static int centerX = 0;
static int centerY = 0;
static int lastX = 0;
static int lastY = 0;

void kinc_internal_mouse_trigger_press(int window, int button, int x, int y) {
	lastX = x;
	lastY = y;
	if (mouse_press_callback != NULL) {
		mouse_press_callback(window, button, x, y, mouse_press_callback_data);
	}
}

void kinc_internal_mouse_trigger_move(int window, int x, int y) {
	int movementX = 0;
	int movementY = 0;
	if (kinc_mouse_is_locked()) {
		movementX = x - centerX;
		movementY = y - centerY;
		if (movementX != 0 || movementY != 0) {
			kinc_mouse_set_position(window, centerX, centerY);
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
		mouse_move_callback(window, x, y, movementX, movementY, mouse_move_callback_data);
	}
}

bool kinc_mouse_is_locked(void) {
	return locked;
}

void kinc_mouse_lock(int window) {
	if (!kinc_mouse_can_lock()) {
		return;
	}
	locked = true;
	kinc_internal_mouse_lock(window);
	kinc_mouse_get_position(window, &preLockX, &preLockY);
	centerX = kinc_window_width(window) / 2;
	centerY = kinc_window_height(window) / 2;
	kinc_mouse_set_position(window, centerX, centerY);
}

void kinc_mouse_unlock(void) {
	if (!kinc_mouse_can_lock()) {
		return;
	}
	moved = false;
	locked = false;
	kinc_internal_mouse_unlock();
	kinc_mouse_set_position(preLockWindow, preLockX, preLockY);
}

#endif

#ifdef __cplusplus
}
#endif
