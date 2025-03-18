#include "util.h"

#include "../array.h"
#include "../errors.h"

#include <string.h>

void indent(char *code, size_t *offset, int indentation) {
	indentation = indentation < 15 ? indentation : 15;
	char str[16];
	memset(str, '\t', sizeof(str));
	str[indentation] = 0;
	*offset += sprintf(&code[*offset], "%s", str);
}

uint32_t base_type_size(type_id type) {
	if (type == float_id) {
		return 4;
	}
	if (type == float2_id) {
		return 4 * 2;
	}
	if (type == float3_id) {
		return 4 * 3;
	}
	if (type == float4_id) {
		return 4 * 4;
	}
	if (type == float4x4_id) {
		return 4 * 4 * 4;
	}
	if (type == float3x3_id) {
		return 3 * 4 * 4;
	}

	if (type == uint_id) {
		return 4;
	}
	if (type == uint2_id) {
		return 4 * 2;
	}
	if (type == uint3_id) {
		return 4 * 3;
	}
	if (type == uint4_id) {
		return 4 * 4;
	}

	if (type == int_id) {
		return 4;
	}
	if (type == int2_id) {
		return 4 * 2;
	}
	if (type == int3_id) {
		return 4 * 3;
	}
	if (type == int4_id) {
		return 4 * 4;
	}

	debug_context context = {0};
	error(context, "Unknown type %s for structure", get_name(get_type(type)->name));
	return 1;
}

uint32_t struct_size(type_id id) {
	uint32_t size = 0;
	type    *t    = get_type(id);
	for (size_t member_index = 0; member_index < t->members.size; ++member_index) {
		size += base_type_size(t->members.m[member_index].type.type);
	}
	return size;
}
