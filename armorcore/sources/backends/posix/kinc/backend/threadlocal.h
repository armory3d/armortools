#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	pthread_key_t key;
} kinc_thread_local_impl_t;

#ifdef __cplusplus
}
#endif
