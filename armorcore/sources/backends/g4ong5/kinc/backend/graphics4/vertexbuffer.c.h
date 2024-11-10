#include <kinc/backend/graphics4/vertexbuffer.h>

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/vertexbuffer.h>

extern uint64_t frameNumber;
extern bool waitAfterNextDraw;

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                int instanceDataStepRate) {
	int multiple = usage == KINC_G4_USAGE_STATIC ? 1 : 500;
	kinc_g5_vertex_buffer_init(&buffer->impl._buffer, count * multiple, structure, usage == KINC_G4_USAGE_STATIC, instanceDataStepRate);
	buffer->impl._multiple = multiple;
	buffer->impl._lastFrameNumber = 0;
	buffer->impl._currentIndex = 0;
	buffer->impl.myCount = count;
}

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_destroy(&buffer->impl._buffer);
}

static void prepareLock(kinc_g4_vertex_buffer_t *buffer) {
	/*if (frameNumber > _lastFrameNumber) {
	    _lastFrameNumber = frameNumber;
	    _currentIndex = 0;
	}
	else {*/
	++buffer->impl._currentIndex;
	if (buffer->impl._currentIndex >= buffer->impl._multiple - 1) {
		waitAfterNextDraw = true;
	}
	if (buffer->impl._currentIndex >= buffer->impl._multiple) {
		buffer->impl._currentIndex = 0;
	}
	//}
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	prepareLock(buffer);
	return kinc_g5_vertex_buffer_lock(&buffer->impl._buffer, buffer->impl._currentIndex * kinc_g4_vertex_buffer_count(buffer),
	                                  kinc_g4_vertex_buffer_count(buffer));
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	prepareLock(buffer);
	return kinc_g5_vertex_buffer_lock(&buffer->impl._buffer, start + buffer->impl._currentIndex * kinc_g4_vertex_buffer_count(buffer), count);
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_unlock_all(&buffer->impl._buffer);
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g5_vertex_buffer_unlock(&buffer->impl._buffer, count);
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_stride(&buffer->impl._buffer);
}
