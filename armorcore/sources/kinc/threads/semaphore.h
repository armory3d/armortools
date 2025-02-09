#pragma once

#include <kinc/global.h>
#include <kinc/backend/semaphore.h>
#include <stdbool.h>

/*! \file semaphore.h
    \brief A semaphore is a fancier version of an event that includes a counter to control how many threads are allowed to work on a task.
*/

typedef struct kinc_semaphore {
	kinc_semaphore_impl_t impl;
} kinc_semaphore_t;

void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max);
void kinc_semaphore_destroy(kinc_semaphore_t *semaphore);
void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count);
void kinc_semaphore_acquire(kinc_semaphore_t *semaphore);
bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double timeout);
