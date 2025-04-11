#ifndef KONG_TYPES_HEADER
#define KONG_TYPES_HEADER

#include "names.h"
#include "tokenizer.h"

#include <stdbool.h>
#include <stddef.h>

#define NO_TYPE 0xFFFFFFFF

typedef uint32_t type_id;

typedef struct unresolved_type_ref {
	name_id  name;
	uint32_t array_size;
} unresolved_type_ref;

typedef struct type_ref {
	type_id             type;
	unresolved_type_ref unresolved;
} type_ref;

void init_type_ref(type_ref *t, name_id name);

typedef struct member {
	name_id  name;
	type_ref type;
	token    value;
} member;

#define MAX_MEMBERS 1024

typedef struct members {
	member m[MAX_MEMBERS];
	size_t size;
} members;

typedef struct attribute {
	name_id name;
	double  parameters[16];
	uint8_t paramters_count;
} attribute;

typedef struct attributes {
	attribute attributes[64];
	uint8_t   attributes_count;
} attribute_list;

bool has_attribute(attribute_list *attributes, name_id name);

attribute *find_attribute(attribute_list *attributes, name_id name);

typedef struct type {
	attribute_list attributes;
	name_id        name;
	bool           built_in;

	members members;

	type_id  base;
	uint32_t array_size;
} type;

void types_init(void);

type_id add_type(name_id name);

type_id find_type_by_name(name_id name);

type_id find_type_by_ref(type_ref *t);

type *get_type(type_id t);

extern type_id void_id;
extern type_id float_id;
extern type_id float2_id;
extern type_id float3_id;
extern type_id float4_id;
extern type_id float3x3_id;
extern type_id float4x4_id;
extern type_id int_id;
extern type_id int2_id;
extern type_id int3_id;
extern type_id int4_id;
extern type_id uint_id;
extern type_id uint2_id;
extern type_id uint3_id;
extern type_id uint4_id;
extern type_id bool_id;
extern type_id bool2_id;
extern type_id bool3_id;
extern type_id bool4_id;
extern type_id tex2d_type_id;
extern type_id tex2darray_type_id;
extern type_id texcube_type_id;
extern type_id sampler_type_id;
extern type_id ray_type_id;
extern type_id bvh_type_id;

static inline bool is_texture(type_id id) {
	if (id == tex2d_type_id || id == tex2darray_type_id || id == texcube_type_id) {
		return true;
	}

	type *t = get_type(id);

	return t->base == tex2d_type_id || t->base == tex2darray_type_id || t->base == texcube_type_id;
}

static inline bool is_cbv_srv_uav(type_id t) {
	return is_texture(t) || t == bvh_type_id || !get_type(t)->built_in;
}

static inline bool is_sampler(type_id t) {
	return t == sampler_type_id;
}

typedef struct swizzle {
	uint32_t indices[4];
	uint32_t size;
} swizzle;

bool is_vector_or_scalar(type_id t);

bool is_vector(type_id t);

uint32_t vector_size(type_id t);

type_id vector_base_type(type_id vector_type);

#endif
