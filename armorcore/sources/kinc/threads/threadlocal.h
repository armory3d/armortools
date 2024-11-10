#pragma once

#include <kinc/global.h>

#include <kinc/backend/threadlocal.h>

/*! \file threadlocal.h
    \brief Provides storage-slots for thread-specific data.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_thread_local {
	kinc_thread_local_impl_t impl;
} kinc_thread_local_t;

/// <summary>
/// Initializes a thread-specific storage-slot.
/// </summary>
/// <param name="local">The storage-slot to initialize</param>
void kinc_thread_local_init(kinc_thread_local_t *local);

/// <summary>
/// Destroys a storage-slot.
/// </summary>
/// <param name="local">The storage-slot to destroy</param>
void kinc_thread_local_destroy(kinc_thread_local_t *local);

/// <summary>
/// Gets the data in the storage-slot.
/// </summary>
/// <param name="local">The slot to query</param>
void *kinc_thread_local_get(kinc_thread_local_t *local);

/// <summary>
/// Sets the data in the storage-slot.
/// </summary>
/// <param name="local">The slot to put the data into</param>
/// <param name="data">The data to put in the slot</param>
void kinc_thread_local_set(kinc_thread_local_t *local, void *data);

#ifdef __cplusplus
}
#endif
