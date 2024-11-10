#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <kinc/backend/graphics4/ShaderHash.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t hash;
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_internal_shader_constant_t;

typedef struct {
	kinc_internal_shader_constant_t constants[64];
	int constantsSize;
	kinc_internal_hash_index_t attributes[64];
	kinc_internal_hash_index_t textures[64];
	void *shader;
	uint8_t *data;
	int length;
	int type;
} kinc_g4_shader_impl_t;

typedef struct {
	uint32_t vertexOffset;
	uint32_t vertexSize;
	uint32_t fragmentOffset;
	uint32_t fragmentSize;
	uint32_t geometryOffset;
	uint32_t geometrySize;
	uint32_t tessEvalOffset;
	uint32_t tessEvalSize;
	uint32_t tessControlOffset;
	uint32_t tessControlSize;
	uint32_t computeOffset;
	uint32_t computeSize;
	uint8_t vertexColumns;
	uint8_t vertexRows;
	uint8_t fragmentColumns;
	uint8_t fragmentRows;
	uint8_t geometryColumns;
	uint8_t geometryRows;
	uint8_t tessEvalColumns;
	uint8_t tessEvalRows;
	uint8_t tessControlColumns;
	uint8_t tessControlRows;
	uint8_t computeColumns;
	uint8_t computeRows;
} kinc_g4_constant_location_impl_t;

#ifdef __cplusplus
}
#endif
