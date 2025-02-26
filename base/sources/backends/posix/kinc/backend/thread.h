#pragma once

#include <pthread.h>

typedef struct {
	void *param;
	void (*thread)(void *param);
	pthread_t pthread;
} kinc_thread_impl_t;
