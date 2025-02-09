#pragma once

typedef struct {
	void *fiber;
	void (*func)(void *param);
	void *param;
} kinc_fiber_impl_t;
