#pragma once

typedef struct kinc_g5_compute_shader_impl {
	char name[1024];
	void *_function;
	void *_pipeline;
	void *_reflection;
} kinc_g5_compute_shader_impl;
