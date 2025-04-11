#include "types.h"

#include "errors.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static type   *types           = NULL;
static type_id types_size      = 1024;
/*static*/ type_id next_type_index = 0;

type_id void_id;
type_id float_id;
type_id float2_id;
type_id float3_id;
type_id float4_id;
type_id float2x2_id;
type_id float3x2_id;
type_id float2x3_id;
type_id float4x2_id;
type_id float2x4_id;
type_id float3x3_id;
type_id float4x3_id;
type_id float3x4_id;
type_id float4x4_id;
type_id int_id;
type_id int2_id;
type_id int3_id;
type_id int4_id;
type_id uint_id;
type_id uint2_id;
type_id uint3_id;
type_id uint4_id;
type_id bool_id;
type_id bool2_id;
type_id bool3_id;
type_id bool4_id;
type_id function_type_id;
type_id tex2d_type_id;
type_id tex2darray_type_id;
type_id texcube_type_id;
type_id sampler_type_id;
type_id ray_type_id;
type_id bvh_type_id;

typedef struct prefix {
	char   str[5];
	size_t size;
} prefix;

void init_type_ref(type_ref *t, name_id name) {
	t->type                  = NO_TYPE;
	t->unresolved.name       = name;
	t->unresolved.array_size = 0;
}

void types_init(void) {
	type         *new_types = realloc(types, types_size * sizeof(type));
	debug_context context   = {0};
	check(new_types != NULL, context, "Could not allocate types");
	types           = new_types;
	next_type_index = 0;

	void_id                     = add_type(add_name("void"));
	get_type(void_id)->built_in = true;

	sampler_type_id                        = add_type(add_name("sampler"));
	get_type(sampler_type_id)->built_in    = true;
	tex2d_type_id                          = add_type(add_name("tex2d"));
	get_type(tex2d_type_id)->built_in      = true;
	tex2darray_type_id                     = add_type(add_name("tex2darray"));
	get_type(tex2darray_type_id)->built_in = true;
	texcube_type_id                        = add_type(add_name("texcube"));
	get_type(texcube_type_id)->built_in    = true;

	bool_id                      = add_type(add_name("bool"));
	get_type(bool_id)->built_in  = true;
	float_id                     = add_type(add_name("float"));
	get_type(float_id)->built_in = true;
	int_id                       = add_type(add_name("int"));
	get_type(int_id)->built_in   = true;
	uint_id                      = add_type(add_name("uint"));
	get_type(uint_id)->built_in  = true;

	float2_id                     = add_type(add_name("float2"));
	get_type(float2_id)->built_in = true;

	float3_id                     = add_type(add_name("float3"));
	get_type(float3_id)->built_in = true;

	float4_id                     = add_type(add_name("float4"));
	get_type(float4_id)->built_in = true;

	int2_id                     = add_type(add_name("int2"));
	get_type(int2_id)->built_in = true;

	int3_id                     = add_type(add_name("int3"));
	get_type(int3_id)->built_in = true;

	int4_id                     = add_type(add_name("int4"));
	get_type(int4_id)->built_in = true;

	uint2_id                     = add_type(add_name("uint2"));
	get_type(uint2_id)->built_in = true;

	uint3_id                     = add_type(add_name("uint3"));
	get_type(uint3_id)->built_in = true;

	uint4_id                     = add_type(add_name("uint4"));
	get_type(uint4_id)->built_in = true;

	bool2_id                     = add_type(add_name("bool2"));
	get_type(bool2_id)->built_in = true;

	bool3_id                     = add_type(add_name("bool3"));
	get_type(bool3_id)->built_in = true;

	bool4_id                     = add_type(add_name("bool4"));
	get_type(bool4_id)->built_in = true;

	float2x2_id                     = add_type(add_name("float2x2"));
	get_type(float2x2_id)->built_in = true;

	float3x2_id                     = add_type(add_name("float3x2"));
	get_type(float3x2_id)->built_in = true;

	float2x3_id                     = add_type(add_name("float2x3"));
	get_type(float2x3_id)->built_in = true;

	float4x2_id                     = add_type(add_name("float4x2"));
	get_type(float4x2_id)->built_in = true;

	float2x4_id                     = add_type(add_name("float2x4"));
	get_type(float2x4_id)->built_in = true;

	float3x3_id                     = add_type(add_name("float3x3"));
	get_type(float3x3_id)->built_in = true;

	float4x3_id                     = add_type(add_name("float4x3"));
	get_type(float4x3_id)->built_in = true;

	float3x4_id                     = add_type(add_name("float3x4"));
	get_type(float3x4_id)->built_in = true;

	float4x4_id                     = add_type(add_name("float4x4"));
	get_type(float4x4_id)->built_in = true;

	{
		ray_type_id                     = add_type(add_name("ray"));
		get_type(ray_type_id)->built_in = true;

		type *t = get_type(ray_type_id);

		t->members.m[t->members.size].name      = add_name("origin");
		t->members.m[t->members.size].type.type = float3_id;
		++t->members.size;

		t->members.m[t->members.size].name      = add_name("direction");
		t->members.m[t->members.size].type.type = float3_id;
		++t->members.size;

		t->members.m[t->members.size].name      = add_name("min");
		t->members.m[t->members.size].type.type = float_id;
		++t->members.size;

		t->members.m[t->members.size].name      = add_name("max");
		t->members.m[t->members.size].type.type = float_id;
		++t->members.size;
	}

	{
		bvh_type_id                     = add_type(add_name("bvh"));
		get_type(bvh_type_id)->built_in = true;
	}

	{
		function_type_id                     = add_type(add_name("fun"));
		get_type(function_type_id)->built_in = true;
	}
}

static void grow_if_needed(uint64_t size) {
	while (size >= types_size) {
		types_size *= 2;
		type         *new_types = realloc(types, types_size * sizeof(type));
		debug_context context   = {0};
		check(new_types != NULL, context, "Could not allocate types");
		types = new_types;
	}
}

type_id add_type(name_id name) {
	grow_if_needed(next_type_index + 1);

	type_id s = next_type_index;
	++next_type_index;

	types[s].name                        = name;
	types[s].attributes.attributes_count = 0;
	types[s].members.size                = 0;
	types[s].built_in                    = false;
	types[s].array_size                  = 0;
	types[s].base                        = NO_TYPE;

	return s;
}

type_id find_type_by_name(name_id name) {
	debug_context context = {0};
	check(name != NO_NAME, context, "Attempted to find a no-name");
	for (type_id i = 0; i < next_type_index; ++i) {
		if (types[i].name == name) {
			return i;
		}
	}

	return NO_TYPE;
}

type_id find_type_by_ref(type_ref *t) {
	if (t->type != NO_TYPE) {
		return t->type;
	}

	debug_context context = {0};
	check(t->unresolved.name != NO_NAME, context, "Attempted to find a no-name");

	type_id base_type_id = NO_TYPE;

	for (type_id i = 0; i < next_type_index; ++i) {
		if (types[i].name == t->unresolved.name) {
			if (types[i].array_size == t->unresolved.array_size) {
				return i;
			}
			base_type_id = i;
		}
	}

	if (base_type_id != NO_TYPE) {
		type_id new_type_id  = add_type(t->unresolved.name);
		type   *new_type     = get_type(new_type_id);
		new_type->array_size = t->unresolved.array_size;

		type *base_type    = get_type(base_type_id);
		new_type->base     = base_type_id;
		new_type->built_in = base_type->built_in;

		return new_type_id;
	}

	return NO_TYPE;
}

type *get_type(type_id s) {
	if (s >= next_type_index) {
		return NULL;
	}
	return &types[s];
}

bool has_attribute(attribute_list *attributes, name_id name) {
	for (uint8_t index = 0; index < attributes->attributes_count; ++index) {
		if (attributes->attributes[index].name == name) {
			return true;
		}
	}
	return false;
}

attribute *find_attribute(attribute_list *attributes, name_id name) {
	for (uint8_t index = 0; index < attributes->attributes_count; ++index) {
		if (attributes->attributes[index].name == name) {
			return &attributes->attributes[index];
		}
	}
	return NULL;
}

bool is_vector_or_scalar(type_id t) {
	return t == float_id || t == float2_id || t == float3_id || t == float4_id || t == int_id || t == int2_id || t == int3_id || t == int4_id || t == uint_id ||
	       t == uint2_id || t == uint3_id || t == uint4_id || t == bool_id || t == bool2_id || t == bool3_id || t == bool4_id;
}

bool is_vector(type_id t) {
	return t == float2_id || t == float3_id || t == float4_id || t == int2_id || t == int3_id || t == int4_id || t == uint2_id || t == uint3_id ||
	       t == uint4_id || t == bool2_id || t == bool3_id || t == bool4_id;
}

uint32_t vector_size(type_id t) {
	if (t == float_id || t == int_id || t == uint_id || t == bool_id) {
		return 1u;
	}

	if (t == float2_id || t == int2_id || t == uint2_id || t == bool2_id) {
		return 2u;
	}

	if (t == float3_id || t == int3_id || t == uint3_id || t == bool3_id) {
		return 3u;
	}

	if (t == float4_id || t == int4_id || t == uint4_id || t == bool4_id) {
		return 4u;
	}

	assert(false);
	return 0;
}

type_id vector_base_type(type_id vector_type) {
	if (vector_type == float2_id || vector_type == float3_id || vector_type == float4_id) {
		return float_id;
	}
	if (vector_type == int2_id || vector_type == int3_id || vector_type == int4_id) {
		return int_id;
	}
	if (vector_type == uint2_id || vector_type == uint3_id || vector_type == uint4_id) {
		return uint_id;
	}
	if (vector_type == bool2_id || vector_type == bool3_id || vector_type == bool4_id) {
		return bool_id;
	}

	assert(false);
	return float_id;
}
