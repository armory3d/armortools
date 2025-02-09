#pragma once

#include <pthread.h>

typedef struct {
	pthread_cond_t event;
	pthread_mutex_t mutex;
	volatile bool set;
	bool auto_reset;
} kinc_event_impl_t;
