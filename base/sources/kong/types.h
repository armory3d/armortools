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

typedef enum texture_kind {
	TEXTURE_KIND_NONE,
	TEXTURE_KIND_1D,
	TEXTURE_KIND_2D,
	TEXTURE_KIND_3D,
	TEXTURE_KIND_1D_ARRAY,
	TEXTURE_KIND_2D_ARRAY,
	TEXTURE_KIND_CUBE,
	TEXTURE_KIND_CUBE_ARRAY,
} texture_kind;

typedef enum texture_format {
	TEXTURE_FORMAT_UNDEFINED,
	TEXTURE_FORMAT_R8_UNORM,
	TEXTURE_FORMAT_R8_SNORM,
	TEXTURE_FORMAT_R8_UINT,
	TEXTURE_FORMAT_R8_SINT,
	TEXTURE_FORMAT_R16_UINT,
	TEXTURE_FORMAT_R16_SINT,
	TEXTURE_FORMAT_R16_FLOAT,
	TEXTURE_FORMAT_RG8_UNORM,
	TEXTURE_FORMAT_RG8_SNORM,
	TEXTURE_FORMAT_RG8_UINT,
	TEXTURE_FORMAT_RG8_SINT,
	TEXTURE_FORMAT_R32_UINT,
	TEXTURE_FORMAT_R32_SINT,
	TEXTURE_FORMAT_R32_FLOAT,
	TEXTURE_FORMAT_RG16_UINT,
	TEXTURE_FORMAT_RG16_SINT,
	TEXTURE_FORMAT_RG16_FLOAT,
	TEXTURE_FORMAT_RGBA8_UNORM,
	TEXTURE_FORMAT_RGBA8_UNORM_SRGB,
	TEXTURE_FORMAT_RGBA8_SNORM,
	TEXTURE_FORMAT_RGBA8_UINT,
	TEXTURE_FORMAT_RGBA8_SINT,
	TEXTURE_FORMAT_BGRA8_UNORM,
	TEXTURE_FORMAT_BGRA8_UNORM_SRGB,
	TEXTURE_FORMAT_RGB9E5U_FLOAT,
	TEXTURE_FORMAT_RGB10A2_UINT,
	TEXTURE_FORMAT_RGB10A2_UNORM,
	TEXTURE_FORMAT_RG11B10U_FLOAT,
	TEXTURE_FORMAT_RG32_UINT,
	TEXTURE_FORMAT_RG32_SINT,
	TEXTURE_FORMAT_RG32_FLOAT,
	TEXTURE_FORMAT_RGBA16_UINT,
	TEXTURE_FORMAT_RGBA16_SINT,
	TEXTURE_FORMAT_RGBA16_FLOAT,
	TEXTURE_FORMAT_RGBA32_UINT,
	TEXTURE_FORMAT_RGBA32_SINT,
	TEXTURE_FORMAT_RGBA32_FLOAT,
	// TEXTURE_FORMAT_STENCIL8, // not available in d3d12
	TEXTURE_FORMAT_DEPTH16_UNORM,
	TEXTURE_FORMAT_DEPTH24_NOTHING8,
	TEXTURE_FORMAT_DEPTH24_STENCIL8,
	TEXTURE_FORMAT_DEPTH32_FLOAT,
	TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_NOTHING24,

	TEXTURE_FORMAT_DEPTH,
	TEXTURE_FORMAT_FRAMEBUFFER,
} texture_format;

bool is_depth(texture_format format);

typedef struct type {
	attribute_list attributes;
	name_id        name;
	bool           built_in;

	members members;

	type_id  base;
	uint32_t array_size;

	texture_kind   tex_kind;
	texture_format tex_format;
} type;

void types_init(void);

type_id add_type(name_id name);

type_id add_full_type(type *t);

type_id find_type_by_name(name_id name);

type_id find_type_by_ref(type_ref *t);

type *get_type(type_id t);

extern type_id void_id;
extern type_id float_id;
extern type_id float2_id;
extern type_id float3_id;
extern type_id float4_id;
extern type_id float2x2_id;
extern type_id float3x2_id;
extern type_id float2x3_id;
extern type_id float4x2_id;
extern type_id float2x4_id;
extern type_id float3x3_id;
extern type_id float4x3_id;
extern type_id float3x4_id;
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
extern type_id sampler_type_id;
extern type_id ray_type_id;
extern type_id bvh_type_id;

static inline bool is_texture(type_id id) {
	while (id != NO_TYPE) {
		type *t = get_type(id);

		if (t->tex_kind != TEXTURE_KIND_NONE) {
			return true;
		}

		id = t->base;
	}

	return false;
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

bool is_matrix(type_id t);

uint32_t vector_size(type_id t);

type_id vector_base_type(type_id vector_type);

type_id vector_to_size(type_id vector_type, uint32_t size);

#endif
