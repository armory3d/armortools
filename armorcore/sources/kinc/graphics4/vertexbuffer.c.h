#include "vertexbuffer.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/vertexbuffer.h>

static void init_vertex_element(kinc_g4_vertex_element_t *element, const char *name, kinc_g4_vertex_data_t data) {
	element->name = name;
	element->data = data;
}

void kinc_g4_vertex_structure_init(kinc_g4_vertex_structure_t *structure) {
	structure->size = 0;
}

void kinc_g4_vertex_structure_add(kinc_g4_vertex_structure_t *structure, const char *name, kinc_g4_vertex_data_t data) {
	init_vertex_element(&structure->elements[structure->size++], name, data);
}

extern bool waitAfterNextDraw;

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage) {
	buffer->impl._multiple = usage == KINC_G4_USAGE_STATIC ? 1 : 2;
	buffer->impl._currentIndex = 0;
	buffer->impl.myCount = count;
	for (int i = 0; i < buffer->impl._multiple; ++i) {
		kinc_g5_vertex_buffer_init(&buffer->impl._buffer[i], count, structure, usage == KINC_G4_USAGE_STATIC);
	}
}

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	for (int i = 0; i < buffer->impl._multiple; ++i) {
		kinc_g5_vertex_buffer_destroy(&buffer->impl._buffer);
	}
}

static void prepareLock(kinc_g4_vertex_buffer_t *buffer) {
	++buffer->impl._currentIndex;
	if (buffer->impl._currentIndex >= buffer->impl._multiple - 1) {
		waitAfterNextDraw = true;
	}
	if (buffer->impl._currentIndex >= buffer->impl._multiple) {
		buffer->impl._currentIndex = 0;
	}
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	prepareLock(buffer);
	return kinc_g5_vertex_buffer_lock_all(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	prepareLock(buffer);
	return kinc_g5_vertex_buffer_lock(&buffer->impl._buffer[buffer->impl._currentIndex], start, count);
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g5_vertex_buffer_unlock_all(&buffer->impl._buffer[buffer->impl._currentIndex]);
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_g5_vertex_buffer_unlock(&buffer->impl._buffer[buffer->impl._currentIndex], count);
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_stride(&buffer->impl._buffer[buffer->impl._currentIndex]);
}
