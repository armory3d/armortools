#pragma once

typedef struct {
	void *metal_buffer;
	int count;
	bool gpu_memory;
	int last_start;
	int last_count;
} IndexBuffer5Impl;
