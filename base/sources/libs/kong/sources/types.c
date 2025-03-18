#include "types.h"

#include "errors.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static type   *types           = NULL;
static type_id types_size      = 1024;
static type_id next_type_index = 0;

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

static void permute_for_real(const char *set, prefix p, int n, int k, void (*found)(char *)) {
	if (k == 0) {
		found(p.str);
		return;
	}

	for (int i = 0; i < n; ++i) {
		prefix newPrefix              = p;
		newPrefix.str[newPrefix.size] = set[i];
		++newPrefix.size;
		permute_for_real(set, newPrefix, n, k - 1, found);
	}
}

static void permute(const char *set, int n, int k, void (*found)(char *)) {
	prefix prefix;
	memset(prefix.str, 0, sizeof(prefix.str));
	prefix.size = 0;
	permute_for_real(set, prefix, n, k, found);
}

static void vec2_found_f32(char *permutation) {
	type         *t       = get_type(float2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float_id;
	++t->members.size;
}

static void vec2_found_vec2(char *permutation) {
	type         *t       = get_type(float2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float2_id;
	++t->members.size;
}

static void vec3_found_f32(char *permutation) {
	type         *t       = get_type(float3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float_id;
	++t->members.size;
}

static void vec3_found_vec2(char *permutation) {
	type         *t       = get_type(float3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float2_id;
	++t->members.size;
}

static void vec3_found_vec3(char *permutation) {
	type         *t       = get_type(float3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float3_id;
	++t->members.size;
}

static void vec4_found_f32(char *permutation) {
	type         *t       = get_type(float4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float_id;
	++t->members.size;
}

static void vec4_found_vec2(char *permutation) {
	type         *t       = get_type(float4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float2_id;
	++t->members.size;
}

static void vec4_found_vec3(char *permutation) {
	type         *t       = get_type(float4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float3_id;
	++t->members.size;
}

static void vec4_found_vec4(char *permutation) {
	type         *t       = get_type(float4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = float4_id;
	++t->members.size;
}

static void int2_found_int(char *permutation) {
	type         *t       = get_type(int2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int_id;
	++t->members.size;
}

static void int2_found_int2(char *permutation) {
	type         *t       = get_type(int2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int2_id;
	++t->members.size;
}

static void int3_found_int(char *permutation) {
	type         *t       = get_type(int3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int_id;
	++t->members.size;
}

static void int3_found_int2(char *permutation) {
	type         *t       = get_type(int3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int2_id;
	++t->members.size;
}

static void int3_found_int3(char *permutation) {
	type         *t       = get_type(int3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int3_id;
	++t->members.size;
}

static void int4_found_int(char *permutation) {
	type         *t       = get_type(int4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int_id;
	++t->members.size;
}

static void int4_found_int2(char *permutation) {
	type         *t       = get_type(int4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int2_id;
	++t->members.size;
}

static void int4_found_int3(char *permutation) {
	type         *t       = get_type(int4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int3_id;
	++t->members.size;
}

static void int4_found_int4(char *permutation) {
	type         *t       = get_type(int4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = int4_id;
	++t->members.size;
}

static void uint2_found_uint(char *permutation) {
	type         *t       = get_type(uint2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint_id;
	++t->members.size;
}

static void uint2_found_uint2(char *permutation) {
	type         *t       = get_type(uint2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint2_id;
	++t->members.size;
}

static void uint3_found_uint(char *permutation) {
	type         *t       = get_type(uint3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint_id;
	++t->members.size;
}

static void uint3_found_uint2(char *permutation) {
	type         *t       = get_type(uint3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint2_id;
	++t->members.size;
}

static void uint3_found_uint3(char *permutation) {
	type         *t       = get_type(uint3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint3_id;
	++t->members.size;
}

static void uint4_found_uint(char *permutation) {
	type         *t       = get_type(uint4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint_id;
	++t->members.size;
}

static void uint4_found_uint2(char *permutation) {
	type         *t       = get_type(uint4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint2_id;
	++t->members.size;
}

static void uint4_found_uint3(char *permutation) {
	type         *t       = get_type(uint4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint3_id;
	++t->members.size;
}

static void uint4_found_uint4(char *permutation) {
	type         *t       = get_type(uint4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = uint4_id;
	++t->members.size;
}

static void bool2_found_bool(char *permutation) {
	type         *t       = get_type(bool2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool_id;
	++t->members.size;
}

static void bool2_found_bool2(char *permutation) {
	type         *t       = get_type(bool2_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool2_id;
	++t->members.size;
}

static void bool3_found_bool(char *permutation) {
	type         *t       = get_type(bool3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool_id;
	++t->members.size;
}

static void bool3_found_bool2(char *permutation) {
	type         *t       = get_type(bool3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool2_id;
	++t->members.size;
}

static void bool3_found_bool3(char *permutation) {
	type         *t       = get_type(bool3_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool3_id;
	++t->members.size;
}

static void bool4_found_bool(char *permutation) {
	type         *t       = get_type(bool4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool_id;
	++t->members.size;
}

static void bool4_found_bool2(char *permutation) {
	type         *t       = get_type(bool4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool2_id;
	++t->members.size;
}

static void bool4_found_bool3(char *permutation) {
	type         *t       = get_type(bool4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool3_id;
	++t->members.size;
}

static void bool4_found_bool4(char *permutation) {
	type         *t       = get_type(bool4_id);
	debug_context context = {0};
	check(t->members.size < MAX_MEMBERS, context, "Out of members");
	t->members.m[t->members.size].name      = add_name(permutation);
	t->members.m[t->members.size].type.type = bool4_id;
	++t->members.size;
}

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

	{
		float2_id                     = add_type(add_name("float2"));
		get_type(float2_id)->built_in = true;
		const char *letters           = "xy";
		permute(letters, (int)strlen(letters), 1, vec2_found_f32);
		permute(letters, (int)strlen(letters), 2, vec2_found_vec2);
		letters = "rg";
		permute(letters, (int)strlen(letters), 1, vec2_found_f32);
		permute(letters, (int)strlen(letters), 2, vec2_found_vec2);
	}

	{
		float3_id                     = add_type(add_name("float3"));
		get_type(float3_id)->built_in = true;
		const char *letters           = "xyz";
		permute(letters, (int)strlen(letters), 1, vec3_found_f32);
		permute(letters, (int)strlen(letters), 2, vec3_found_vec2);
		permute(letters, (int)strlen(letters), 3, vec3_found_vec3);
		letters = "rgb";
		permute(letters, (int)strlen(letters), 1, vec3_found_f32);
		permute(letters, (int)strlen(letters), 2, vec3_found_vec2);
		permute(letters, (int)strlen(letters), 3, vec3_found_vec3);
	}

	{
		float4_id                     = add_type(add_name("float4"));
		get_type(float4_id)->built_in = true;
		const char *letters           = "xyzw";
		permute(letters, (int)strlen(letters), 1, vec4_found_f32);
		permute(letters, (int)strlen(letters), 2, vec4_found_vec2);
		permute(letters, (int)strlen(letters), 3, vec4_found_vec3);
		permute(letters, (int)strlen(letters), 4, vec4_found_vec4);
		letters = "rgba";
		permute(letters, (int)strlen(letters), 1, vec4_found_f32);
		permute(letters, (int)strlen(letters), 2, vec4_found_vec2);
		permute(letters, (int)strlen(letters), 3, vec4_found_vec3);
		permute(letters, (int)strlen(letters), 4, vec4_found_vec4);
	}

	{
		int2_id                     = add_type(add_name("int2"));
		get_type(int2_id)->built_in = true;
		const char *letters         = "xy";
		permute(letters, (int)strlen(letters), 1, int2_found_int);
		permute(letters, (int)strlen(letters), 2, int2_found_int2);
		letters = "rg";
		permute(letters, (int)strlen(letters), 1, int2_found_int);
		permute(letters, (int)strlen(letters), 2, int2_found_int2);
	}

	{
		int3_id                     = add_type(add_name("int3"));
		get_type(int3_id)->built_in = true;
		const char *letters         = "xyz";
		permute(letters, (int)strlen(letters), 1, int3_found_int);
		permute(letters, (int)strlen(letters), 2, int3_found_int2);
		permute(letters, (int)strlen(letters), 3, int3_found_int3);
		letters = "rgb";
		permute(letters, (int)strlen(letters), 1, int3_found_int);
		permute(letters, (int)strlen(letters), 2, int3_found_int2);
		permute(letters, (int)strlen(letters), 3, int3_found_int3);
	}

	{
		int4_id                     = add_type(add_name("int4"));
		get_type(int4_id)->built_in = true;
		const char *letters         = "xyzw";
		permute(letters, (int)strlen(letters), 1, int4_found_int);
		permute(letters, (int)strlen(letters), 2, int4_found_int2);
		permute(letters, (int)strlen(letters), 3, int4_found_int3);
		permute(letters, (int)strlen(letters), 4, int4_found_int4);
		letters = "rgba";
		permute(letters, (int)strlen(letters), 1, int4_found_int);
		permute(letters, (int)strlen(letters), 2, int4_found_int2);
		permute(letters, (int)strlen(letters), 3, int4_found_int3);
		permute(letters, (int)strlen(letters), 4, int4_found_int4);
	}

	{
		uint2_id                     = add_type(add_name("uint2"));
		get_type(uint2_id)->built_in = true;
		const char *letters          = "xy";
		permute(letters, (int)strlen(letters), 1, uint2_found_uint);
		permute(letters, (int)strlen(letters), 2, uint2_found_uint2);
		letters = "rg";
		permute(letters, (int)strlen(letters), 1, uint2_found_uint);
		permute(letters, (int)strlen(letters), 2, uint2_found_uint2);
	}

	{
		uint3_id                     = add_type(add_name("uint3"));
		get_type(uint3_id)->built_in = true;
		const char *letters          = "xyz";
		permute(letters, (int)strlen(letters), 1, uint3_found_uint);
		permute(letters, (int)strlen(letters), 2, uint3_found_uint2);
		permute(letters, (int)strlen(letters), 3, uint3_found_uint3);
		letters = "rgb";
		permute(letters, (int)strlen(letters), 1, uint3_found_uint);
		permute(letters, (int)strlen(letters), 2, uint3_found_uint2);
		permute(letters, (int)strlen(letters), 3, uint3_found_uint3);
	}

	{
		uint4_id                     = add_type(add_name("uint4"));
		get_type(uint4_id)->built_in = true;
		const char *letters          = "xyzw";
		permute(letters, (int)strlen(letters), 1, uint4_found_uint);
		permute(letters, (int)strlen(letters), 2, uint4_found_uint2);
		permute(letters, (int)strlen(letters), 3, uint4_found_uint3);
		permute(letters, (int)strlen(letters), 4, uint4_found_uint4);
		letters = "rgba";
		permute(letters, (int)strlen(letters), 1, uint4_found_uint);
		permute(letters, (int)strlen(letters), 2, uint4_found_uint2);
		permute(letters, (int)strlen(letters), 3, uint4_found_uint3);
		permute(letters, (int)strlen(letters), 4, uint4_found_uint4);
	}

	{
		bool2_id                     = add_type(add_name("bool2"));
		get_type(bool2_id)->built_in = true;
		const char *letters          = "xy";
		permute(letters, (int)strlen(letters), 1, bool2_found_bool);
		permute(letters, (int)strlen(letters), 2, bool2_found_bool2);
		letters = "rg";
		permute(letters, (int)strlen(letters), 1, bool2_found_bool);
		permute(letters, (int)strlen(letters), 2, bool2_found_bool2);
	}

	{
		bool3_id                     = add_type(add_name("bool3"));
		get_type(bool3_id)->built_in = true;
		const char *letters          = "xyz";
		permute(letters, (int)strlen(letters), 1, bool3_found_bool);
		permute(letters, (int)strlen(letters), 2, bool3_found_bool2);
		permute(letters, (int)strlen(letters), 3, bool3_found_bool3);
		letters = "rgb";
		permute(letters, (int)strlen(letters), 1, bool3_found_bool);
		permute(letters, (int)strlen(letters), 2, bool3_found_bool2);
		permute(letters, (int)strlen(letters), 3, bool3_found_bool3);
	}

	{
		bool4_id                     = add_type(add_name("bool4"));
		get_type(bool4_id)->built_in = true;
		const char *letters          = "xyzw";
		permute(letters, (int)strlen(letters), 1, bool4_found_bool);
		permute(letters, (int)strlen(letters), 2, bool4_found_bool2);
		permute(letters, (int)strlen(letters), 3, bool4_found_bool3);
		permute(letters, (int)strlen(letters), 4, bool4_found_bool4);
		letters = "rgba";
		permute(letters, (int)strlen(letters), 1, bool4_found_bool);
		permute(letters, (int)strlen(letters), 2, bool4_found_bool2);
		permute(letters, (int)strlen(letters), 3, bool4_found_bool3);
		permute(letters, (int)strlen(letters), 4, bool4_found_bool4);
	}

	{
		float2x2_id                     = add_type(add_name("float2x2"));
		get_type(float2x2_id)->built_in = true;
	}

	{
		float3x2_id                     = add_type(add_name("float3x2"));
		get_type(float3x2_id)->built_in = true;
	}

	{
		float2x3_id                     = add_type(add_name("float2x3"));
		get_type(float2x3_id)->built_in = true;
	}

	{
		float4x2_id                     = add_type(add_name("float4x2"));
		get_type(float4x2_id)->built_in = true;
	}

	{
		float2x4_id                     = add_type(add_name("float2x4"));
		get_type(float2x4_id)->built_in = true;
	}

	{
		float3x3_id                     = add_type(add_name("float3x3"));
		get_type(float3x3_id)->built_in = true;
	}

	{
		float4x3_id                     = add_type(add_name("float4x3"));
		get_type(float4x3_id)->built_in = true;
	}

	{
		float3x4_id                     = add_type(add_name("float3x4"));
		get_type(float3x4_id)->built_in = true;
	}

	{
		float4x4_id                     = add_type(add_name("float4x4"));
		get_type(float4x4_id)->built_in = true;
	}

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

	bool found_name = false;

	for (type_id i = 0; i < next_type_index; ++i) {
		if (types[i].name == t->unresolved.name) {
			found_name = true;
			if (types[i].array_size == t->unresolved.array_size) {
				return i;
			}
		}
	}

	if (found_name) {
		type_id new_type               = add_type(t->unresolved.name);
		get_type(new_type)->array_size = t->unresolved.array_size;

		type_ref no_array_type;
		no_array_type                       = *t;
		no_array_type.unresolved.array_size = 0;
		get_type(new_type)->base            = find_type_by_ref(&no_array_type);

		return new_type;
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
