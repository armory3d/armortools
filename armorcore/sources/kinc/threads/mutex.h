#pragma once

#include <kinc/global.h>

#include <kinc/backend/mutex.h>

#include <stdbool.h>

/*! \file mutex.h
    \brief Provides mutexes which are used to synchronize threads and uber-mutexes which are used to synchronize processes.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_mutex {
	kinc_mutex_impl_t impl;
} kinc_mutex_t;

/// <summary>
/// Initializes a mutex-object.
/// </summary>
/// <param name="mutex">The mutex to initialize</param>
void kinc_mutex_init(kinc_mutex_t *mutex);

/// <summary>
/// Destroys a mutex-object.
/// </summary>
/// <param name="mutex">The mutex to destroy</param>
void kinc_mutex_destroy(kinc_mutex_t *mutex);

/// <summary>
/// Locks a mutex. A mutex can only be locked from one thread - when other threads attempt to lock the mutex the function will only return once the mutex has
/// been unlocked.
/// </summary>
/// <param name="mutex">The mutex to lock</param>
void kinc_mutex_lock(kinc_mutex_t *mutex);

/// <summary>
/// Attempts to lock the mutex which will only succeed if no other thread currently holds the lock.
/// </summary>
/// <param name="mutex">The mutex to lock</param>
/// <returns>Whether the mutex could be locked</returns>
bool kinc_mutex_try_to_lock(kinc_mutex_t *mutex);

/// <summary>
/// Unlocks the mutex which then allows other threads to lock it.
/// </summary>
/// <param name="mutex">The mutex to unlock</param>
void kinc_mutex_unlock(kinc_mutex_t *mutex);

typedef struct kinc_uber_mutex {
	kinc_uber_mutex_impl_t impl;
} kinc_uber_mutex_t;

/// <summary>
/// Initializes an uber-mutex-object.
/// </summary>
/// <param name="mutex">The uber-mutex to initialize</param>
/// <param name="name">A name assigned to the uber-mutex - uber-mutex-creation fails if an uber-mutex of that name already exists</param>
/// <returns>Whether the uber-mutex could be created</returns>
bool kinc_uber_mutex_init(kinc_uber_mutex_t *mutex, const char *name);

/// <summary>
/// Destroys an uber-mutex-obejct.
/// </summary>
/// <param name="mutex">The uber-mutex to destroy</param>
void kinc_uber_mutex_destroy(kinc_uber_mutex_t *mutex);

/// <summary>
/// Locks an uber-mutex.
/// </summary>
/// <param name="mutex">The uber-mutex to lock</param>
void kinc_uber_mutex_lock(kinc_uber_mutex_t *mutex);

/// <summary>
/// Unlocks an uber-mutex.
/// </summary>
/// <param name="mutex">The uber-mutex to unlock</param>
/// <returns></returns>
void kinc_uber_mutex_unlock(kinc_uber_mutex_t *mutex);

#ifdef __cplusplus
}
#endif
