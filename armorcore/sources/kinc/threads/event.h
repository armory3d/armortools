#pragma once

#include <kinc/global.h>
#include <kinc/backend/event.h>
#include <stdbool.h>

/*! \file event.h
    \brief An event is a simple threading-object that allows a thread to wait for something to happen on another thread.
*/

typedef struct kinc_event {
	kinc_event_impl_t impl;
} kinc_event_t;

void kinc_event_init(kinc_event_t *event, bool auto_clear);
void kinc_event_destroy(kinc_event_t *event);
void kinc_event_signal(kinc_event_t *event);
void kinc_event_wait(kinc_event_t *event);
bool kinc_event_try_to_wait(kinc_event_t *event, double timeout);
void kinc_event_reset(kinc_event_t *event);
