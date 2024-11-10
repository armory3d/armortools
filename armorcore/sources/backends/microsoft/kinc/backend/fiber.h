#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *fiber;
	void (*func)(void *param);
	void *param;
} kinc_fiber_impl_t;

#ifdef __cplusplus
}
#endif
