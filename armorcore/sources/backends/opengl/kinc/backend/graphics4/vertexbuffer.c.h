#include "ogl.h"
#include <kinc/backend/graphics4/shader.h>
#include <kinc/backend/graphics4/vertexbuffer.h>

#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/vertexbuffer.h>

#include <assert.h>
#include <stdlib.h>

extern kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer;
static kinc_g4_vertex_buffer_t *currentVertexBuffer = NULL;

#if defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18
void *glesVertexAttribDivisor;
#endif

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int vertexCount, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                int instanceDataStepRate) {
	buffer->impl.myCount = vertexCount;
	buffer->impl.instanceDataStepRate = instanceDataStepRate;
#ifndef NDEBUG
	buffer->impl.initialized = false;
#endif
	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g4_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += kinc_g4_vertex_data_size(element.data);
	}
	buffer->impl.structure = *structure;

	unsigned gl_usage;
	switch (usage) {
	case KINC_G4_USAGE_STATIC:
	default:
		gl_usage = GL_STATIC_DRAW;
		break;
	case KINC_G4_USAGE_DYNAMIC:
		gl_usage = GL_DYNAMIC_DRAW;
		break;
	case KINC_G4_USAGE_READABLE:
		gl_usage = GL_DYNAMIC_DRAW;
		break;
	}

	glGenBuffers(1, &buffer->impl.bufferId);
	glCheckErrors();
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	glBufferData(GL_ARRAY_BUFFER, buffer->impl.myStride * buffer->impl.myCount, NULL, gl_usage);
	glCheckErrors();
	buffer->impl.data = (float *)malloc(vertexCount * buffer->impl.myStride);
}

void Kinc_Internal_G4_VertexBuffer_Unset(kinc_g4_vertex_buffer_t *buffer);

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	Kinc_Internal_G4_VertexBuffer_Unset(buffer);
	glDeleteBuffers(1, &buffer->impl.bufferId);
	free(buffer->impl.data);
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	buffer->impl.sectionStart = 0;
	buffer->impl.sectionSize = buffer->impl.myCount * buffer->impl.myStride;
	return buffer->impl.data;
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	buffer->impl.sectionStart = start * buffer->impl.myStride;
	buffer->impl.sectionSize = count * buffer->impl.myStride;
	uint8_t *u8data = (uint8_t *)buffer->impl.data;
	return (float *)&u8data[buffer->impl.sectionStart];
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	uint8_t *u8data = (uint8_t *)buffer->impl.data;
	glBufferSubData(GL_ARRAY_BUFFER, buffer->impl.sectionStart, buffer->impl.sectionSize, u8data + buffer->impl.sectionStart);
	glCheckErrors();
#ifndef NDEBUG
	buffer->impl.initialized = true;
#endif
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	uint8_t *u8data = (uint8_t *)buffer->impl.data;
	glBufferSubData(GL_ARRAY_BUFFER, buffer->impl.sectionStart, count * buffer->impl.myStride, u8data + buffer->impl.sectionStart);
	glCheckErrors();
#ifndef NDEBUG
	buffer->impl.initialized = true;
#endif
}

int Kinc_G4_Internal_SetVertexAttributes(kinc_g4_vertex_buffer_t *buffer, int offset);

int kinc_internal_g4_vertex_buffer_set(kinc_g4_vertex_buffer_t *buffer, int offset) {
	// assert(buffer->impl.initialized); // Vertex Buffer is used before lock/unlock was called
	int offsetoffset = Kinc_G4_Internal_SetVertexAttributes(buffer, offset);
	if (Kinc_Internal_CurrentIndexBuffer != NULL) {
		kinc_internal_g4_index_buffer_set(Kinc_Internal_CurrentIndexBuffer);
	}
	return offsetoffset;
}

void Kinc_Internal_G4_VertexBuffer_Unset(kinc_g4_vertex_buffer_t *buffer) {
	if (currentVertexBuffer == buffer) {
		currentVertexBuffer = NULL;
	}
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

#if !defined(KINC_OPENGL_ES) || (defined(KINC_OPENGL_ES) && defined(KINC_WASM)) || (defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18)
static bool attribDivisorUsed = false;
#endif

int Kinc_G4_Internal_SetVertexAttributes(kinc_g4_vertex_buffer_t *buffer, int offset) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();

	int internaloffset = 0;
	int actualIndex = 0;
	for (int index = 0; index < buffer->impl.structure.size; ++index) {
		kinc_g4_vertex_element_t element = buffer->impl.structure.elements[index];
		int size = 0;
		GLenum type = GL_FLOAT;
		switch (element.data) {
		case KINC_G4_VERTEX_DATA_NONE:
			break;
		case KINC_G4_VERTEX_DATA_F32_1X:
			size = 1;
			break;
		case KINC_G4_VERTEX_DATA_F32_2X:
			size = 2;
			break;
		case KINC_G4_VERTEX_DATA_F32_3X:
			size = 3;
			break;
		case KINC_G4_VERTEX_DATA_F32_4X:
			size = 4;
			break;
		case KINC_G4_VERTEX_DATA_F32_4X4:
			size = 16;
			break;
		case KINC_G4_VERTEX_DATA_I8_1X:
		case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
			size = 1;
			type = GL_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_U8_1X:
		case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
			size = 1;
			type = GL_UNSIGNED_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_I8_2X:
		case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
			size = 2;
			type = GL_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_U8_2X:
		case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
			size = 2;
			type = GL_UNSIGNED_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_I8_4X:
		case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
			size = 4;
			type = GL_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_U8_4X:
		case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
			size = 4;
			type = GL_UNSIGNED_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_I16_1X:
		case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
			size = 1;
			type = GL_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_U16_1X:
		case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
			size = 1;
			type = GL_UNSIGNED_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_I16_2X:
		case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
			size = 2;
			type = GL_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_U16_2X:
		case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
			size = 2;
			type = GL_UNSIGNED_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_I16_4X:
		case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
			size = 4;
			type = GL_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_U16_4X:
		case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
			size = 4;
			type = GL_UNSIGNED_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_I32_1X:
			size = 1;
			type = GL_INT;
			break;
		case KINC_G4_VERTEX_DATA_U32_1X:
			size = 1;
			type = GL_UNSIGNED_INT;
			break;
		case KINC_G4_VERTEX_DATA_I32_2X:
			size = 2;
			type = GL_INT;
			break;
		case KINC_G4_VERTEX_DATA_U32_2X:
			size = 2;
			type = GL_UNSIGNED_INT;
			break;
		case KINC_G4_VERTEX_DATA_I32_3X:
			size = 3;
			type = GL_INT;
			break;
		case KINC_G4_VERTEX_DATA_U32_3X:
			size = 3;
			type = GL_UNSIGNED_INT;
			break;
		case KINC_G4_VERTEX_DATA_I32_4X:
			size = 4;
			type = GL_INT;
			break;
		case KINC_G4_VERTEX_DATA_U32_4X:
			size = 4;
			type = GL_UNSIGNED_INT;
			break;
		}
		if (size > 4) {
			int subsize = size;
			int addonOffset = 0;
			while (subsize > 0) {
				glEnableVertexAttribArray(offset + actualIndex);
				glCheckErrors();
				glVertexAttribPointer(offset + actualIndex, 4, type, false, buffer->impl.myStride, (void *)(int64_t)(internaloffset + addonOffset));
				glCheckErrors();
#if !defined(KINC_OPENGL_ES) || (defined(KINC_OPENGL_ES) && defined(KINC_WASM))
				if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
					attribDivisorUsed = true;
					glVertexAttribDivisor(offset + actualIndex, buffer->impl.instanceDataStepRate);
					glCheckErrors();
				}
#endif
#if (defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18)
				if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
					attribDivisorUsed = true;
					((void (*)(GLuint, GLuint))glesVertexAttribDivisor)(offset + actualIndex, buffer->impl.instanceDataStepRate);
					glCheckErrors();
				}
#endif
				subsize -= 4;
				addonOffset += 4 * 4;
				++actualIndex;
			}
		}
		else {
			glEnableVertexAttribArray(offset + actualIndex);
			glCheckErrors();
			glVertexAttribPointer(offset + actualIndex, size, type, type == GL_FLOAT ? false : true, buffer->impl.myStride, (void *)(int64_t)internaloffset);
			glCheckErrors();
#if !defined(KINC_OPENGL_ES) || (defined(KINC_OPENGL_ES) && defined(KINC_WASM))
			if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
				attribDivisorUsed = true;
				glVertexAttribDivisor(offset + actualIndex, buffer->impl.instanceDataStepRate);
				glCheckErrors();
			}
#endif
#if (defined(KINC_OPENGL_ES) && defined(KINC_ANDROID) && KINC_ANDROID_API >= 18)
			if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
				attribDivisorUsed = true;
				((void (*)(GLuint, GLuint))glesVertexAttribDivisor)(offset + actualIndex, buffer->impl.instanceDataStepRate);
				glCheckErrors();
			}
#endif
			++actualIndex;
		}
		internaloffset += kinc_g4_vertex_data_size(element.data);
	}
	int count = kinc_internal_opengl_max_vertex_attribute_arrays - offset;
	for (int index = actualIndex; index < count; ++index) {
		glDisableVertexAttribArray(offset + index);
		glCheckErrors();
	}
	return actualIndex;
}
