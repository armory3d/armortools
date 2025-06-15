#include "globals.h"

#include "errors.h"

#include <assert.h>

static global    globals[1024];
////
// static global_id globals_size = 0;
global_id        globals_size = 0;
////

void globals_init(void) {
	global_value int_value;
	int_value.kind = GLOBAL_VALUE_INT;

	attribute_list attributes = {0};

	int_value.value.ints[0] = 0;
	add_global_with_value(float_id, attributes, add_name("COMPARE_ALWAYS"), int_value);
	int_value.value.ints[0] = 1;
	add_global_with_value(float_id, attributes, add_name("COMPARE_NEVER"), int_value);
	int_value.value.ints[0] = 2;
	add_global_with_value(float_id, attributes, add_name("COMPARE_EQUAL"), int_value);
	int_value.value.ints[0] = 3;
	add_global_with_value(float_id, attributes, add_name("COMPARE_NOT_EQUAL"), int_value);
	int_value.value.ints[0] = 4;
	add_global_with_value(float_id, attributes, add_name("COMPARE_LESS"), int_value);
	int_value.value.ints[0] = 5;
	add_global_with_value(float_id, attributes, add_name("COMPARE_LESS_EQUAL"), int_value);
	int_value.value.ints[0] = 6;
	add_global_with_value(float_id, attributes, add_name("COMPARE_GREATER"), int_value);
	int_value.value.ints[0] = 7;
	add_global_with_value(float_id, attributes, add_name("COMPARE_GREATER_EQUAL"), int_value);

	int_value.value.ints[0] = 0;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ZERO"), int_value);
	int_value.value.ints[0] = 1;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ONE"), int_value);
	int_value.value.ints[0] = 2;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_SRC"), int_value);
	int_value.value.ints[0] = 3;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ONE_MINUS_SRC"), int_value);
	int_value.value.ints[0] = 4;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_SRC_ALPHA"), int_value);
	int_value.value.ints[0] = 5;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ONE_MINUS_SRC_ALPHA"), int_value);
	int_value.value.ints[0] = 6;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_DST"), int_value);
	int_value.value.ints[0] = 7;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ONE_MINUS_DST"), int_value);
	int_value.value.ints[0] = 8;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_DST_ALPHA"), int_value);
	int_value.value.ints[0] = 9;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ONE_MINUS_DST_ALPHA"), int_value);
	int_value.value.ints[0] = 10;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_SRC_ALPHA_SATURATED"), int_value);
	int_value.value.ints[0] = 11;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_CONSTANT"), int_value);
	int_value.value.ints[0] = 12;
	add_global_with_value(float_id, attributes, add_name("BLEND_FACTOR_ONE_MINUS_CONSTANT"), int_value);

	int_value.value.ints[0] = 0;
	add_global_with_value(float_id, attributes, add_name("BLEND_OPERATION_ADD"), int_value);
	int_value.value.ints[0] = 1;
	add_global_with_value(float_id, attributes, add_name("BLEND_OPERATION_SUBTRACT"), int_value);
	int_value.value.ints[0] = 2;
	add_global_with_value(float_id, attributes, add_name("BLEND_OPERATION_REVERSE_SUBTRACT"), int_value);
	int_value.value.ints[0] = 3;
	add_global_with_value(float_id, attributes, add_name("BLEND_OPERATION_MIN"), int_value);
	int_value.value.ints[0] = 4;
	add_global_with_value(float_id, attributes, add_name("BLEND_OPERATION_MAX"), int_value);

	global_value uint_value;
	uint_value.kind = GLOBAL_VALUE_UINT;

	uint_value.value.uints[0] = 0;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R8_UNORM"), uint_value);
	uint_value.value.uints[0] = 1;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R8_SNORM"), uint_value);
	uint_value.value.uints[0] = 2;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R8_UINT"), uint_value);
	uint_value.value.uints[0] = 3;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R8_SINT"), uint_value);
	uint_value.value.uints[0] = 4;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R16_UINT"), uint_value);
	uint_value.value.uints[0] = 5;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R16_SINT"), uint_value);
	uint_value.value.uints[0] = 6;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R16_FLOAT"), uint_value);
	uint_value.value.uints[0] = 7;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG8_UNORM"), uint_value);
	uint_value.value.uints[0] = 8;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG8_SNORM"), uint_value);
	uint_value.value.uints[0] = 9;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG8_UINT"), uint_value);
	uint_value.value.uints[0] = 10;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG8_SINT"), uint_value);
	uint_value.value.uints[0] = 11;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R32_UINT"), uint_value);
	uint_value.value.uints[0] = 12;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R32_SINT"), uint_value);
	uint_value.value.uints[0] = 13;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_R32_FLOAT"), uint_value);
	uint_value.value.uints[0] = 14;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG16_UINT"), uint_value);
	uint_value.value.uints[0] = 15;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG16_SINT"), uint_value);
	uint_value.value.uints[0] = 16;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG16_FLOAT"), uint_value);
	uint_value.value.uints[0] = 17;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA8_UNORM"), uint_value);
	uint_value.value.uints[0] = 18;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA8_UNORM_SRGB"), uint_value);
	uint_value.value.uints[0] = 19;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA8_SNORM"), uint_value);
	uint_value.value.uints[0] = 20;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA8_UINT"), uint_value);
	uint_value.value.uints[0] = 21;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA8_SINT"), uint_value);
	uint_value.value.uints[0] = 22;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_BGRA8_UNORM"), uint_value);
	uint_value.value.uints[0] = 23;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_BGRA8_UNORM_SRGB"), uint_value);
	uint_value.value.uints[0] = 24;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGB9E5U_FLOAT"), uint_value);
	uint_value.value.uints[0] = 25;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGB10A2_UINT"), uint_value);
	uint_value.value.uints[0] = 26;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGB10A2_UNORM"), uint_value);
	uint_value.value.uints[0] = 27;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG11B10U_FLOAT"), uint_value);
	uint_value.value.uints[0] = 28;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG32_UINT"), uint_value);
	uint_value.value.uints[0] = 29;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG32_SINT"), uint_value);
	uint_value.value.uints[0] = 30;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RG32_FLOAT"), uint_value);
	uint_value.value.uints[0] = 31;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA16_UINT"), uint_value);
	uint_value.value.uints[0] = 32;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA16_SINT"), uint_value);
	uint_value.value.uints[0] = 33;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA16_FLOAT"), uint_value);
	uint_value.value.uints[0] = 34;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA32_UINT"), uint_value);
	uint_value.value.uints[0] = 35;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA32_SINT"), uint_value);
	uint_value.value.uints[0] = 36;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_RGBA32_FLOAT"), uint_value);
	uint_value.value.uints[0] = 37;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_DEPTH16_UNORM"), uint_value);
	uint_value.value.uints[0] = 38;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_DEPTH24_NOTHING8"), uint_value);
	uint_value.value.uints[0] = 39;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_DEPTH24_STENCIL8"), uint_value);
	uint_value.value.uints[0] = 40;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_DEPTH32_FLOAT"), uint_value);
	uint_value.value.uints[0] = 41;
	add_global_with_value(uint_id, attributes, add_name("TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_NOTHING24"), uint_value);
}

global_id add_global(type_id type, attribute_list attributes, name_id name) {
	uint32_t index            = globals_size;
	globals[index].name       = name;
	globals[index].type       = type;
	globals[index].var_index  = 0;
	globals[index].value.kind = GLOBAL_VALUE_NONE;
	globals[index].attributes = attributes;
	globals[index].sets_count = 0;
	globals[index].usage      = 0;
	globals_size += 1;
	return index;
}

global_id add_global_with_value(type_id type, attribute_list attributes, name_id name, global_value value) {
	uint32_t index            = globals_size;
	globals[index].name       = name;
	globals[index].type       = type;
	globals[index].var_index  = 0;
	globals[index].value      = value;
	globals[index].attributes = attributes;
	globals[index].sets_count = 0;
	globals[index].usage      = 0;
	globals_size += 1;
	return index;
}

bool global_has_usage(global_id g, global_usage usage) {
	return (get_global(g)->usage & usage) == usage;
}

global *find_global(name_id name) {
	for (uint32_t i = 0; i < globals_size; ++i) {
		if (globals[i].name == name) {
			return &globals[i];
		}
	}

	return NULL;
}

global *get_global(global_id id) {
	if (id >= globals_size) {
		return NULL;
	}

	return &globals[id];
}

void assign_global_var(global_id id, uint64_t var_index) {
	debug_context context = {0};
	check(id < globals_size, context, "Encountered a global with a weird id");
	globals[id].var_index = var_index;
}
