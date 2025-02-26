#pragma once

#include <pthread.h>

typedef struct {
	pthread_mutex_t mutex;
} kinc_mutex_impl_t;

typedef struct {
	int nothing;
} kinc_uber_mutex_impl_t;
