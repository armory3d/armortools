#include "analyzer.h"
#include "compiler.h"
#include "disasm.h"
#include "errors.h"
#include "functions.h"
#include "globals.h"
#include "log.h"
#include "names.h"
#include "parser.h"
#include "tokenizer.h"
#include "transformer.h"
#include "typer.h"
#include "types.h"

#include "backends/hlsl.h"
#include "backends/metal.h"
#include "backends/spirv.h"
#include "backends/wgsl.h"

#include "dir.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum arg_mode { MODE_MODECHECK, MODE_INPUT, MODE_OUTPUT, MODE_PLATFORM, MODE_API, MODE_INTEGRATION } arg_mode;

static void help(void) {
	printf("No help is coming.");
}

static void read_file(char *filename) {
	FILE *file = fopen(filename, "rb");

	if (file == NULL) {
		kong_log(LOG_LEVEL_ERROR, "File %s not found.", filename);
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *data = (char *)malloc(size + 1);

	debug_context context = {0};
	context.filename      = filename;
	check(data != NULL, context, "Could not allocate memory to read file %s", filename);

	fread(data, 1, size, file);
	data[size] = 0;

	fclose(file);

	tokens tokens = tokenize(filename, data);

	free(data);

	parse(filename, &tokens);
}

typedef enum integration_kind { INTEGRATION_KORE3 } integration_kind;

#include "analyzer.h"
#include "backends/hlsl.h"
#include "backends/metal.h"
#include "backends/spirv.h"
#include "backends/wgsl.h"
#include "compiler.h"
#include "disasm.h"
#include "errors.h"
#include "functions.h"
#include "globals.h"
#include "libs/stb_ds.h"
#include "log.h"
#include "names.h"
#include "parser.h"
#include "tokenizer.h"
#include "transformer.h"
#include "typer.h"
#include "types.h"

extern uint64_t    next_variable_id;
extern size_t      allocated_globals_size;
extern function_id next_function_index;
extern global_id   globals_size;
extern name_id     names_index;
extern size_t      sets_count;
extern type_id     next_type_index;
extern size_t      vertex_inputs_size;
extern size_t      fragment_inputs_size;
extern size_t      vertex_functions_size;
extern size_t      fragment_functions_size;
extern struct {
	char   *key;
	name_id value;
}          *hash;
extern int  expression_index;
extern int  statement_index;
extern bool kong_error;

uint64_t    _next_variable_id;
size_t      _allocated_globals_size;
function_id _next_function_index;
global_id   _globals_size;
name_id     _names_index;
size_t      _sets_count;
type_id     _next_type_index;
size_t      _vertex_inputs_size;
size_t      _fragment_inputs_size;
size_t      _vertex_functions_size;
size_t      _fragment_functions_size;
struct {
	char   *key;
	name_id value;
}   *_hash;
int  _expression_index;
int  _statement_index;
void hlsl_export2(char **vs, char **fs, api_kind d3d, bool debug);
void spirv_export2(char **vs, char **fs, int *vs_size, int *fs_size, bool debug);
void wgsl_export2(char **vs, char **fs);
void console_info(char *s);

static struct {
	char   *key;
	name_id value;
} *_clone_hash(struct {
	char   *key;
	name_id value;
} * hash) {
	struct {
		char   *key;
		name_id value;
	} *clone = NULL;
	sh_new_arena(clone);
	ptrdiff_t len = shlen(hash);
	for (ptrdiff_t i = 0; i < len; i++) {
		shput(clone, hash[i].key, hash[i].value);
	}
	return clone;
}

void gpu_create_shaders_from_kong(char *kong, char **vs, char **fs, int *vs_size, int *fs_size) {
	static bool first = true;
	if (first) {
		first = false;
		names_init();
		types_init();
		functions_init();
		globals_init();

		_next_variable_id        = next_variable_id;
		_allocated_globals_size  = allocated_globals_size;
		_next_function_index     = next_function_index;
		_globals_size            = globals_size;
		_names_index             = names_index;
		_sets_count              = sets_count;
		_next_type_index         = next_type_index;
		_vertex_inputs_size      = vertex_inputs_size;
		_fragment_inputs_size    = fragment_inputs_size;
		_vertex_functions_size   = vertex_functions_size;
		_fragment_functions_size = fragment_functions_size;
		_hash                    = _clone_hash(hash);
		_expression_index        = expression_index;
		_statement_index         = statement_index;
	}
	else {
		next_variable_id        = _next_variable_id;
		allocated_globals_size  = _allocated_globals_size;
		next_function_index     = _next_function_index;
		globals_size            = _globals_size;
		names_index             = _names_index;
		sets_count              = _sets_count;
		next_type_index         = _next_type_index;
		vertex_inputs_size      = _vertex_inputs_size;
		fragment_inputs_size    = _fragment_inputs_size;
		vertex_functions_size   = _vertex_functions_size;
		fragment_functions_size = _fragment_functions_size;
		shfree(hash);
		hash             = _clone_hash(_hash);
		expression_index = _expression_index;
		statement_index  = _statement_index;
	}

	kong_error    = false;
	char  *from   = "";
	tokens tokens = tokenize(from, kong);
	parse(from, &tokens);
	resolve_types();

	if (kong_error) {
		console_info("Warning: Shader compilation failed");
		free(tokens.t);
#if defined(__APPLE__)
		*vs = "";
		*fs = "";
#endif
		return;
	}
	allocate_globals();
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		compile_function_block(&get_function(i)->code, get_function(i)->block);
	}
	analyze();

#ifdef _WIN32

	hlsl_export2(vs, fs, API_DIRECT3D11, false);

#elif defined(__APPLE__)

	static char vs_temp[1024 * 128];
	strcpy(vs_temp, "//>kong_vert\n");
	char *metal = metal_export("");
	strcat(vs_temp, metal);
	*vs = &vs_temp[0];
	*fs = "//>kong_frag\n";
	free(metal);

#elif defined(IRON_WASM)

	transform(TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE);
	wgsl_export2(vs, fs);

#else

	transform(TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE | TRANSFORM_FLAG_BINARY_UNIFY_LENGTH);
	spirv_export2(vs, fs, vs_size, fs_size, false);

#endif

	free(tokens.t);
}
