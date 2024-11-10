#pragma once

#include <kinc/global.h>

#include <kinc/backend/event.h>

#include <stdbool.h>

/*! \file event.h
    \brief An event is a simple threading-object that allows a thread to wait for something to happen on another thread.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_event {
	kinc_event_impl_t impl;
} kinc_event_t;

/// <summary>
/// Initializes an event-object.
/// </summary>
/// <param name="event">The event to initialize</param>
/// <param name="auto_clear">When auto-clear is true, the event is automatically reset to an unsignaled state after a successful wait-operation</param>
void kinc_event_init(kinc_event_t *event, bool auto_clear);

/// <summary>
/// Destroys an event-object.
/// </summary>
/// <param name="event">The event to destroy</param>
void kinc_event_destroy(kinc_event_t *event);

/// <summary>
/// Signals an event which allows threads which are waiting for the event to continue.
/// </summary>
/// <param name="event">The event to signal</param>
void kinc_event_signal(kinc_event_t *event);

/// <summary>
/// Waits for an event to be signaled.
/// </summary>
/// <param name="event">The event to wait for</param>
void kinc_event_wait(kinc_event_t *event);

/// <summary>
/// Waits for an event to be signaled or the provided timeout to run out - whatever happens first.
/// </summary>
/// <param name="event">The event to wait for</param>
/// <param name="timeout">The timeout in seconds after which the function returns if the event hasn't been signaled</param>
/// <returns>Whether the event has been signaled</returns>
bool kinc_event_try_to_wait(kinc_event_t *event, double timeout);

/// <summary>
/// Resets an event to an unsignaled state.
/// </summary>
/// <param name="event">The event to reset</param>
void kinc_event_reset(kinc_event_t *event);

#ifdef __cplusplus
}
#endif
