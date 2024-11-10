#pragma once

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#ifdef __APPLE__
	dispatch_semaphore_t semaphore;
#else
	sem_t semaphore;
#endif
} kinc_semaphore_impl_t;

#ifdef __cplusplus
}
#endif
