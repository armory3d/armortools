#pragma once

#include <kinc/global.h>

/*! \file gamepad.h
    \brief Provides gamepad-support.
*/

#define KINC_GAMEPAD_MAX_COUNT 12

void kinc_gamepad_set_connect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);
void kinc_gamepad_set_disconnect_callback(void (*value)(int /*gamepad*/, void * /*userdata*/), void *userdata);
void kinc_gamepad_set_axis_callback(void (*value)(int /*gamepad*/, int /*axis*/, float /*value*/, void * /*userdata*/), void *userdata);
void kinc_gamepad_set_button_callback(void (*value)(int /*gamepad*/, int /*button*/, float /*value*/, void * /*userdata*/), void *userdata);
const char *kinc_gamepad_vendor(int gamepad);
const char *kinc_gamepad_product_name(int gamepad);
bool kinc_gamepad_connected(int gamepad);
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
