// Based on https://github.com/Kode/Kongruent by RobDangerous
#pragma once

#include <stdint.h>

#define OPCODES_SIZE (1024 * 1024)

typedef uint32_t type_id;
typedef size_t name_id;

typedef struct attribute {
	name_id name;
	double  parameters[16];
	uint8_t paramters_count;
} attribute;

typedef struct attributes {
	attribute attributes[64];
	uint8_t   attributes_count;
} attribute_list;

typedef struct unresolved_type_ref {
	name_id  name;
	uint32_t array_size;
} unresolved_type_ref;

typedef struct type_ref {
	type_id             type;
	unresolved_type_ref unresolved;
} type_ref;

typedef struct opcodes {
	uint8_t o[OPCODES_SIZE];
	size_t  size;
} opcodes;

typedef struct function {
	attribute_list    attributes;
	name_id           name;
	type_ref          return_type;
	name_id           parameter_names[256];
	type_ref          parameter_types[256];
	name_id           parameter_attributes[256];
	uint8_t           parameters_size;
	struct statement *block;
	opcodes code;
} function;

typedef uint32_t function_id;

// extern type_id void_id;
// extern type_id float_id;
// extern type_id int_id;
// extern type_id uint_id;
// extern type_id bool_id;
