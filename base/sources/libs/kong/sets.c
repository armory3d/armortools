#include "sets.h"

#include "errors.h"

static descriptor_set sets[MAX_SETS];
/*static*/ size_t         sets_count = 0;

descriptor_set *create_set(name_id name) {
	for (size_t set_index = 0; set_index < sets_count; ++set_index) {
		if (sets[set_index].name == name) {
			return &sets[set_index];
		}
	}

	if (sets_count >= MAX_SETS) {
		debug_context context = {0};
		error(context, "Max set count of %i reached", MAX_SETS);
		return NULL;
	}

	descriptor_set *new_set = &sets[sets_count];
	new_set->name           = name;
	new_set->index          = (uint32_t)sets_count;
	new_set->globals.size   = 0;

	sets_count += 1;

	return new_set;
}

descriptor_set *get_set(size_t index) {
	return &sets[index];
}

size_t get_sets_count(void) {
	return sets_count;
}

void add_definition_to_set(descriptor_set *set, definition def) {
	assert(def.kind != DEFINITION_FUNCTION && def.kind != DEFINITION_STRUCT);

	for (size_t global_index = 0; global_index < set->globals.size; ++global_index) {
		if (set->globals.globals[global_index] == def.global) {
			return;
		}
	}

	get_global(def.global)->sets[get_global(def.global)->sets_count] = set;
	get_global(def.global)->sets_count += 1;
	set->globals.globals[set->globals.size] = def.global;
	set->globals.size += 1;
}
