#pragma once

#include <kinc/global.h>
#include <kinc/backend/fiber.h>

/*! \file fiber.h
    \brief The fiber-API is experimental and only supported on a few system.
*/

typedef struct kinc_fiber {
	kinc_fiber_impl_t impl;
} kinc_fiber_t;

void kinc_fiber_init_current_thread(kinc_fiber_t *fiber);
void kinc_fiber_init(kinc_fiber_t *fiber, void (*func)(void *param), void *param);
void kinc_fiber_destroy(kinc_fiber_t *fiber);
void kinc_fiber_switch(kinc_fiber_t *fiber);
