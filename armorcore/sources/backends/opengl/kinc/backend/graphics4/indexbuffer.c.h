#include "ogl.h"

#include <kinc/graphics4/indexbuffer.h>

#include <stdlib.h>

kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer = NULL;

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_index_buffer_format_t format, kinc_g4_usage_t usage) {
	buffer->impl.count = count;
	buffer->impl.format = format;

	glGenBuffers(1, &buffer->impl.buffer_id);
	glCheckErrors();
	if (format == KINC_G4_INDEX_BUFFER_FORMAT_32BIT) {
		buffer->impl.data = malloc(count * sizeof(uint32_t));
	}
	else {
		buffer->impl.data = malloc(count * sizeof(uint16_t));
	}

	if (format == KINC_G4_INDEX_BUFFER_FORMAT_32BIT && kinc_internal_opengl_force_16bit_index_buffer) {
		buffer->impl.converted_data = malloc(count * sizeof(uint16_t));
	}
	else {
		buffer->impl.converted_data = NULL;
	}

	switch (usage) {
	case KINC_G4_USAGE_STATIC:
		buffer->impl.usage = GL_STATIC_DRAW;
		break;
	case KINC_G4_USAGE_DYNAMIC:
		buffer->impl.usage = GL_DYNAMIC_DRAW;
		break;
	case KINC_G4_USAGE_READABLE:
		buffer->impl.usage = GL_DYNAMIC_DRAW;
		break;
	}
}

void Kinc_Internal_IndexBufferUnset(kinc_g4_index_buffer_t *buffer) {
	if (Kinc_Internal_CurrentIndexBuffer == buffer) {
		Kinc_Internal_CurrentIndexBuffer = NULL;
	}
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	Kinc_Internal_IndexBufferUnset(buffer);
	glDeleteBuffers(1, &buffer->impl.buffer_id);
	free(buffer->impl.data);
	buffer->impl.data = NULL;
	free(buffer->impl.converted_data);
	buffer->impl.converted_data = NULL;
}

static int kinc_g4_internal_index_buffer_stride(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_32BIT ? 4 : 2;
}

void *kinc_g4_index_buffer_lock_all(kinc_g4_index_buffer_t *buffer) {
	return kinc_g4_index_buffer_lock(buffer, 0, kinc_g4_index_buffer_count(buffer));
}

void *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer, int start, int count) {
	uint8_t *data = (uint8_t *)buffer->impl.data;
	return &data[start * kinc_g4_internal_index_buffer_stride(buffer)];
}

void kinc_g4_index_buffer_unlock_all(kinc_g4_index_buffer_t *buffer) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.buffer_id);
	glCheckErrors();

	if (buffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_32BIT && kinc_internal_opengl_force_16bit_index_buffer) {
		uint32_t *data = (uint32_t *)buffer->impl.data;
		for (int i = 0; i < buffer->impl.count; ++i) {
			buffer->impl.converted_data[i] = (uint16_t)data[i];
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.count * sizeof(uint16_t), buffer->impl.converted_data, buffer->impl.usage);
		glCheckErrors();
	}
	else {
		GLsizeiptr size =
		    buffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_16BIT ? buffer->impl.count * sizeof(uint16_t) : buffer->impl.count * sizeof(uint32_t);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer->impl.data, buffer->impl.usage);
		glCheckErrors();
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glCheckErrors();
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer, int count) {
	kinc_g4_index_buffer_unlock_all(buffer);
}

void kinc_internal_g4_index_buffer_set(kinc_g4_index_buffer_t *buffer) {
	Kinc_Internal_CurrentIndexBuffer = buffer;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.buffer_id);
	glCheckErrors();
}

int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.count;
}
