#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	pthread_cond_t event;
	pthread_mutex_t mutex;
	volatile bool set;
	bool auto_reset;
} kinc_event_impl_t;

#ifdef __cplusplus
}
#endif
