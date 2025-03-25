#ifndef KONG_BACKENDS_UTIL_HEADER
#define KONG_BACKENDS_UTIL_HEADER

#include "../types.h"

#include <stdint.h>

void indent(char *code, size_t *offset, int indentation);

uint32_t base_type_size(type_id type);

uint32_t struct_size(type_id id);

bool execute_sync(const char *command, uint32_t *exit_code);

#endif
