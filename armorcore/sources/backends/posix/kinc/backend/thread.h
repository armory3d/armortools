#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *param;
	void (*thread)(void *param);
	pthread_t pthread;
} kinc_thread_impl_t;

#ifdef __cplusplus
}
#endif
