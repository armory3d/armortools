#pragma once

#include <kinc/graphics5/vertexbuffer.h>

typedef struct {
	int myCount;
	kinc_g5_vertex_buffer_t _buffer;
	int _currentIndex;
	int _multiple;
} kinc_g4_vertex_buffer_impl_t;
