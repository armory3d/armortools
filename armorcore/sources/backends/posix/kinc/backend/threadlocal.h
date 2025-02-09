#pragma once

#include <pthread.h>

typedef struct {
	pthread_key_t key;
} kinc_thread_local_impl_t;
