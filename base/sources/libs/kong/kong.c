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
#include "typer.h"
#include "types.h"

// #include "backends/cpu.h"
// #include "backends/glsl.h"
#include "backends/hlsl.h"
#include "backends/metal.h"
#include "backends/spirv.h"
#include "backends/wgsl.h"

// #include "integrations/kore3.h"

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

/*int main(int argc, char **argv) {
	arg_mode mode = MODE_MODECHECK;

	char            *inputs[256] = {0};
	size_t           inputs_size = 0;
	char            *platform    = NULL;
	api_kind         api         = API_DEFAULT;
	integration_kind integration = INTEGRATION_KORE3;
	bool             debug       = false;
	char            *output      = NULL;

	for (int i = 1; i < argc; ++i) {
		char *arg = argv[i];
		switch (mode) {
		case MODE_MODECHECK: {
			if (arg[0] == '-') {
				if (arg[1] == '-') {
					if (strcmp(&arg[2], "in") == 0) {
						mode = MODE_INPUT;
					}
					else if (strcmp(&arg[2], "out") == 0) {
						mode = MODE_OUTPUT;
					}
					else if (strcmp(&arg[2], "platform") == 0) {
						mode = MODE_PLATFORM;
					}
					else if (strcmp(&arg[2], "api") == 0) {
						mode = MODE_API;
					}
					else if (strcmp(&arg[2], "integration") == 0) {
						mode = MODE_INTEGRATION;
					}
					else if (strcmp(&arg[2], "debug") == 0) {
						debug = true;
					}
					else if (strcmp(&arg[2], "help") == 0) {
						help();
						return 0;
					}
					else {
						kong_log(LOG_LEVEL_WARNING, "Ignoring unknown parameter %s", arg);
					}
				}
				else {
					bool ok = true;

					if (arg[1] == 0) {
						kong_log(LOG_LEVEL_WARNING, "Ignoring lonely -.");
						ok = false;
					}

					if (arg[2] != 0) {
						kong_log(LOG_LEVEL_WARNING, "Ignoring parameter %s because the format is not right (one - implies just one letter).", arg);
						ok = false;
					}

					if (ok) {
						switch (arg[1]) {
						case 'i':
							mode = MODE_INPUT;
							break;
						case 'o':
							mode = MODE_OUTPUT;
							break;
						case 'p':
							mode = MODE_PLATFORM;
							break;
						case 'a':
							mode = MODE_API;
							break;
						case 'n':
							mode = MODE_INTEGRATION;
							break;
						case 'h':
							help();
							return 0;
						default: {
							kong_log(LOG_LEVEL_WARNING, "Ignoring unknown parameter %s.", arg);
						}
						}
					}
				}
			}
			else {
				kong_log(LOG_LEVEL_WARNING, "Ignoring parameter %s because the syntax is off.", arg);
			}
			break;
		}
		case MODE_INPUT: {
			inputs[inputs_size] = arg;
			inputs_size += 1;
			mode = MODE_MODECHECK;
			break;
		}
		case MODE_OUTPUT: {
			output = arg;
			mode   = MODE_MODECHECK;
			break;
		}
		case MODE_PLATFORM: {
			platform = arg;
			mode     = MODE_MODECHECK;
			break;
		}
		case MODE_API: {
			if (strcmp(arg, "direct3d11") == 0) {
				api = API_DIRECT3D11;
			}
			else if (strcmp(arg, "direct3d12") == 0) {
				api = API_DIRECT3D12;
			}
			else if (strcmp(arg, "opengl") == 0) {
				api = API_OPENGL;
			}
			else if (strcmp(arg, "metal") == 0) {
				api = API_METAL;
			}
			else if (strcmp(arg, "webgpu") == 0) {
				api = API_WEBGPU;
			}
			else if (strcmp(arg, "vulkan") == 0) {
				api = API_VULKAN;
			}
			else if (strcmp(arg, "default") == 0) {
				api = API_DEFAULT;
			}
			else {
				debug_context context = {0};
				error(context, "Unknown API %s", arg);
			}
			mode = MODE_MODECHECK;
			break;
		}
		case MODE_INTEGRATION: {
			if (strcmp(arg, "kore3") == 0) {
				integration = INTEGRATION_KORE3;
			}
			else {
				debug_context context = {0};
				error(context, "Unknown integration %s", arg);
			}
			mode = MODE_MODECHECK;
			break;
		}
		}
	}

	debug_context context = {0};
	check(platform != NULL, context, "platform parameter not found");

	if (api == API_DEFAULT) {
		if (strcmp(platform, "windows") == 0) {
			api = API_DIRECT3D12;
		}
		else if (strcmp(platform, "macos") == 0) {
			api = API_METAL;
		}
		else if (strcmp(platform, "linux") == 0) {
			api = API_VULKAN;
		}
		else if (strcmp(platform, "ios") == 0) {
			api = API_METAL;
		}
		else if (strcmp(platform, "android") == 0) {
			api = API_VULKAN;
		}
		else if (strcmp(platform, "wasm") == 0) {
			api = API_WEBGPU;
		}
	}

	check(mode == MODE_MODECHECK, context, "Wrong parameter syntax");
	check(inputs_size > 0, context, "no input parameters found");
	check(output != NULL, context, "output parameter not found");
	check(api != API_DEFAULT, context, "api parameter not found");

	names_init();
	types_init();
	functions_init();
	globals_init();

	for (size_t i = 0; i < inputs_size; ++i) {
		directory dir = open_dir(inputs[i]);

		file f = read_next_file(&dir);
		while (f.valid) {
			char path[1024];
			strcpy(path, inputs[i]);
			strcat(path, "/");
			strcat(path, f.name);

			size_t length         = strlen(path);
			size_t dotkong_length = strlen(".kong");
			if (length > dotkong_length && strcmp(&path[length - dotkong_length], ".kong") == 0) {
				read_file(path);
			}

			f = read_next_file(&dir);
		}

		close_dir(&dir);
	}

#ifndef NDEBUG
	kong_log(LOG_LEVEL_INFO, "Functions:");
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		kong_log(LOG_LEVEL_INFO, "%s", get_name(get_function(i)->name));
	}
	kong_log(LOG_LEVEL_INFO, "");

	kong_log(LOG_LEVEL_INFO, "Types:");
	for (type_id i = 0; get_type(i) != NULL; ++i) {
		kong_log(LOG_LEVEL_INFO, "%s (%i)", get_name(get_type(i)->name), i);
	}
	kong_log(LOG_LEVEL_INFO, "");
#endif

	resolve_types();

	allocate_globals();
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		compile_function_block(&get_function(i)->code, get_function(i)->block);
	}

	analyze();

#ifndef NDEBUG
	disassemble();
#endif

	switch (api) {
	case API_DIRECT3D11:
	case API_DIRECT3D12:
		hlsl_export(output, api, debug);
		break;
	case API_OPENGL:
		glsl_export(output);
		break;
	case API_METAL:
		metal_export(output);
		break;
	case API_WEBGPU:
		wgsl_export(output);
		break;
	case API_VULKAN: {
		spirv_export(output, debug);
		break;
	}
	default: {
		debug_context context = {0};
		error(context, "Unknown API");
	}
	}

	cpu_export(output);

	switch (integration) {
	case INTEGRATION_KORE3:
		kore3_export(output, api);
		break;
	}

	return 0;
}*/
