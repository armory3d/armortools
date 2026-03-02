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
