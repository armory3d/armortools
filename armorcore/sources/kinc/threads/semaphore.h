#pragma once

#include <kinc/global.h>

#include <kinc/backend/semaphore.h>

#include <stdbool.h>

/*! \file semaphore.h
    \brief A semaphore is a fancier version of an event that includes a counter to control how many threads are allowed to work on a task.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_semaphore {
	kinc_semaphore_impl_t impl;
} kinc_semaphore_t;

/// <summary>
/// Initializes a semaphore.
/// </summary>
/// <param name="semaphore">The semaphore to initialize</param>
/// <param name="current">The current count of the semaphore</param>
/// <param name="max">The maximum allowed count of the semaphore</param>
void kinc_semaphore_init(kinc_semaphore_t *semaphore, int current, int max);

/// <summary>
/// Destroys a semaphore.
/// </summary>
/// <param name="semaphore">The semaphore to destroy</param>
void kinc_semaphore_destroy(kinc_semaphore_t *semaphore);

/// <summary>
/// Increases the current count of the semaphore, therefore allowing more acquires to succeed.
/// </summary>
/// <param name="semaphore">The semaphore to increase the count on</param>
/// <param name="count">The amount by which the count will be increased</param>
void kinc_semaphore_release(kinc_semaphore_t *semaphore, int count);

/// <summary>
/// Decreases the count of the semaphore by one. Blocks until it is possible to decrease the count if it already reached zero.
/// </summary>
/// <param name="semaphore">The semaphore to acquire</param>
void kinc_semaphore_acquire(kinc_semaphore_t *semaphore);

/// <summary>
/// Attempts to decrease the count of the semaphore by one.
/// </summary>
/// <param name="semaphore">The semaphore to acquire</param>
/// <param name="timeout">The timeout in seconds after which the function returns if the semaphore-count could not be decreased</param>
/// <returns>Whether the semaphore-count could be decreased</returns>
bool kinc_semaphore_try_to_acquire(kinc_semaphore_t *semaphore, double timeout);

#ifdef __cplusplus
}
#endif
