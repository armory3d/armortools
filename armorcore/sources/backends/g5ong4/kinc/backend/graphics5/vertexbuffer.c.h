#include <kinc/graphics5/vertexbuffer.h>

#include <stdlib.h>

kinc_g5_vertex_buffer_t *kinc_g5_internal_current_vertex_buffer = NULL;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory, int instanceDataStepRate) {
	buffer->impl.myCount = count;
	buffer->impl.myStart = 0;
	kinc_g4_vertex_buffer_init(&buffer->impl.buffer, count, structure, KINC_G4_USAGE_STATIC, instanceDataStepRate);
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_destroy(&buffer->impl.buffer);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	return kinc_g4_vertex_buffer_lock_all(&buffer->impl.buffer);
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	return kinc_g4_vertex_buffer_lock(&buffer->impl.buffer, start, count);
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_unlock_all(&buffer->impl.buffer);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count) {
	kinc_g4_vertex_buffer_unlock(&buffer->impl.buffer, count);
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return kinc_g4_vertex_buffer_stride(&buffer->impl.buffer);
}
