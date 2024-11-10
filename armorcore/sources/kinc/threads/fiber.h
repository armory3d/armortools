#pragma once

#include <kinc/global.h>

#include <kinc/backend/fiber.h>

/*! \file fiber.h
    \brief The fiber-API is experimental and only supported on a few system.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_fiber {
	kinc_fiber_impl_t impl;
} kinc_fiber_t;

/// <summary>
/// Uses the current thread as a fiber.
/// </summary>
/// <param name="fiber">The fiber-object to initialize using the current thread</param>
void kinc_fiber_init_current_thread(kinc_fiber_t *fiber);

/// <summary>
/// Initializes a fiber.
/// </summary>
/// <param name="fiber">The fiber-object to initialize</param>
/// <param name="func">The function to be run in the fiber-context</param>
/// <param name="param">A parameter to be provided to the fiber-function when it starts running</param>
void kinc_fiber_init(kinc_fiber_t *fiber, void (*func)(void *param), void *param);

/// <summary>
/// Destroys a fiber.
/// </summary>
/// <param name="fiber">The fiber to destroy</param>
void kinc_fiber_destroy(kinc_fiber_t *fiber);

/// <summary>
/// Switch the current thread to a different fiber.
/// </summary>
/// <param name="fiber">The fiber to switch to</param>
void kinc_fiber_switch(kinc_fiber_t *fiber);

#ifdef __cplusplus
}
#endif
