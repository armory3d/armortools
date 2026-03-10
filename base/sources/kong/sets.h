#ifndef KONG_SETS_HEADER
#define KONG_SETS_HEADER

#include "names.h"
#include "parser.h"

#define MAX_SET_DEFINITIONS 32

typedef struct descriptor_set {
	uint32_t     index;
	name_id      name;
	global_array globals;
} descriptor_set;

#define MAX_SETS 256

descriptor_set *create_set(name_id name);

descriptor_set *get_set(size_t index);

size_t get_sets_count(void);

void add_definition_to_set(descriptor_set *set, definition def);

#endif
