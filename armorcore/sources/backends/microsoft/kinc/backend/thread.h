#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *handle;
	void *param;
	void (*func)(void *param);
} kinc_thread_impl_t;

#ifdef __cplusplus
}
#endif
