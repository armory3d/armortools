#include "names.h"

#include "libs/stb_ds.h"

#include "errors.h"

#include <assert.h>

static char   *names       = NULL;
static size_t  names_size  = 1024 * 1024;
////
// static name_id names_index = 1;
name_id        names_index = 1;
////

static struct {
	char   *key;
	name_id value;
} *hash = NULL;

void names_init(void) {
	char         *new_names = realloc(names, names_size);
	debug_context context   = {0};
	check(new_names != NULL, context, "Could not allocate names");
	names    = new_names;
	names[0] = 0; // make NO_NAME a proper string

	sh_new_arena(hash); // TODO: Get rid of this by using indices internally in the hash-map so it can survive grow_if_needed
}

static void grow_if_needed(size_t size) {
	while (size >= names_size) {
		names_size *= 2;
		char         *new_names = realloc(names, names_size);
		debug_context context   = {0};
		check(new_names != NULL, context, "Could not allocate names");
		names = new_names;
	}
}

name_id add_name(char *name) {
	ptrdiff_t old_id_index = shgeti(hash, name);

	if (old_id_index >= 0) {
		return hash[old_id_index].value;
	}

	size_t length = strlen(name);

	grow_if_needed(names_index + length + 1);

	name_id id = names_index;

	memcpy(&names[id], name, length);
	names[id + length] = 0;

	names_index += length + 1;

	shput(hash, &names[id], id);

	return id;
}

char *get_name(name_id id) {
	debug_context context = {0};
	check(id < names_index, context, "Encountered a weird name id");
	return &names[id];
}
