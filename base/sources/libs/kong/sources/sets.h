#ifndef KONG_SETS_HEADER
#define KONG_SETS_HEADER

#include "names.h"
#include "parser.h"

#define MAX_SET_DEFINITIONS 32

typedef struct descriptor_set {
	uint32_t   index;
	name_id    name;
	definition definitions[MAX_SET_DEFINITIONS];
	size_t     definitions_count;
} descriptor_set;

#define MAX_SETS 256

descriptor_set *create_set(name_id name);

void add_definition_to_set(descriptor_set *set, definition def);

#endif
