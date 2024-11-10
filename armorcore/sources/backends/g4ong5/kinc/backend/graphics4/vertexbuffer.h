#pragma once

#include <kinc/graphics5/vertexbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int myCount;
	// void prepareLock();
	kinc_g5_vertex_buffer_t _buffer;
	int _currentIndex;
	int _multiple;
	uint64_t _lastFrameNumber;
} kinc_g4_vertex_buffer_impl_t;

#ifdef __cplusplus
}
#endif
