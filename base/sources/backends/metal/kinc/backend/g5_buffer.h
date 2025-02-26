#pragma once

typedef struct {
	int myCount;
	int myStride;
	void *mtlBuffer;
	bool gpuMemory;
	int lastStart;
	int lastCount;
} VertexBuffer5Impl;

typedef struct {
	void *_buffer;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

typedef struct {
	void *metal_buffer;
	int count;
	bool gpu_memory;
	int last_start;
	int last_count;
} IndexBuffer5Impl;
