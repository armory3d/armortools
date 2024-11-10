#pragma once

#include <kinc/global.h>

/*! \file gamepad.h
    \brief Provides gamepad-support.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_GAMEPAD_MAX_COUNT 12

/// <summary>
/// Sets the gamepad-connect-callback which is called when a gamepad is connected.
/// </summary>
/// <param name="value">The callback</param>
/// <param name="userdata">Userdata you will receive back as the 2nd callback parameter</param>
void kinc_gamepad_set_connect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);

/// <summary>
/// Sets the gamepad-disconnect-callback which is called when a gamepad is disconnected.
/// </summary>
/// <param name="value">The callback</param>
/// <param name="userdata">Userdata you will receive back as the 2nd callback parameter</param>
void kinc_gamepad_set_disconnect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);

/// <summary>
/// Sets the gamepad-axis-callback which is called with data about changing gamepad-sticks.
/// </summary>
/// <param name="value">The callback</param>
/// <param name="userdata">Userdata you will receive back as the 4th callback parameter</param>
void kinc_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/, void * /*userdata*/), void *userdata);

/// <summary>
/// Sets the gamepad-button-callback which is called with data about changing gamepad-buttons.
/// </summary>
/// <param name="value">The callback</param>
/// <param name="userdata">Userdata you will receive back as the 4th callback parameter</param>
void kinc_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/, void * /*userdata*/), void *userdata);

/// <summary>
/// Returns a vendor-name for a gamepad.
/// </summary>
/// <param name="gamepad">The index of the gamepad for which to receive the vendor-name</param>
/// <returns>The vendor-name</returns>
const char *kinc_gamepad_vendor(int gamepad);

/// <summary>
/// Returns a name for a gamepad.
/// </summary>
/// <param name="gamepad">The index of the gamepad for which to receive the name</param>
/// <returns>The gamepad's name</returns>
const char *kinc_gamepad_product_name(int gamepad);

/// <summary>
/// Checks whether a gamepad is connected.
/// </summary>
/// <param name="gamepad">The index of the gamepad which's connection will be checked</param>
/// <returns>Whether a gamepad is connected for the gamepad-index</returns>
bool kinc_gamepad_connected(int gamepad);

/// <summary>
/// Rumbles a gamepad. Careful here because it might just fall off your table.
/// </summary>
/// <param name="gamepad">The index of the gamepad to rumble</param>
/// <param name="left">Rumble-strength for the left motor between 0 and 1</param>
/// <param name="right">Rumble-strength for the right motor between 0 and 1</param>
void kinc_gamepad_rumble(int gamepad, float left, float right);

void kinc_internal_gamepad_trigger_connect(int gamepad);
void kinc_internal_gamepad_trigger_disconnect(int gamepad);
void kinc_internal_gamepad_trigger_axis(int gamepad, int axis, float value);
void kinc_internal_gamepad_trigger_button(int gamepad, int button, float value);

#ifdef KINC_IMPLEMENTATION_INPUT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <memory.h>
#include <stddef.h>

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

#endif

#ifdef __cplusplus
}
#endif
