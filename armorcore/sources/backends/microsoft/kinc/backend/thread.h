#pragma once

typedef struct {
	void *handle;
	void *param;
	void (*func)(void *param);
} kinc_thread_impl_t;
