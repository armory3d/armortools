#pragma once

#include "names.h"
#include "types.h"

typedef uint32_t global_id;

typedef struct global_value {
	enum {
		GLOBAL_VALUE_FLOAT,
		GLOBAL_VALUE_FLOAT2,
		GLOBAL_VALUE_FLOAT3,
		GLOBAL_VALUE_FLOAT4,
		GLOBAL_VALUE_INT,
		GLOBAL_VALUE_INT2,
		GLOBAL_VALUE_INT3,
		GLOBAL_VALUE_INT4,
		GLOBAL_VALUE_UINT,
		GLOBAL_VALUE_UINT2,
		GLOBAL_VALUE_UINT3,
		GLOBAL_VALUE_UINT4,
		GLOBAL_VALUE_BOOL,
		GLOBAL_VALUE_NONE
	} kind;
	union {
		float    floats[4];
		int      ints[4];
		unsigned uints[4];
		bool     b;
	} value;
} global_value;

struct descriptor_set;

typedef enum global_usage {
	GLOBAL_USAGE_TEXTURE_SAMPLE = 0x00000001,
	GLOBAL_USAGE_TEXTURE_READ   = 0x00000002,
	GLOBAL_USAGE_TEXTURE_WRITE  = 0x00000004,
	GLOBAL_USAGE_BUFFER_WRITE   = 0x00000008,
	GLOBAL_USAGE_SAMPLE_DEPTH   = 0x00000010,
} global_usage;

typedef struct global {
	name_id                name;
	type_id                type;
	uint64_t               var_index;
	global_value           value;
	attribute_list         attributes;
	struct descriptor_set *sets[64];
	size_t                 sets_count;
	uint32_t               usage;
} global;

typedef struct global_array {
	global_id globals[256];
	bool      readable[256];
	bool      writable[256];
	size_t    size;
} global_array;

void globals_init(void);

global_id add_global(type_id type, attribute_list attributes, name_id name);
global_id add_global_with_value(type_id type, attribute_list attributes, name_id name, global_value value);
bool      global_has_usage(global_id g, global_usage usage);

global *find_global(name_id name);

global *get_global(global_id id);

void assign_global_var(global_id id, uint64_t var_index);
