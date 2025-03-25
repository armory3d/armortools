#pragma once

#include "../compiler.h"

typedef char *(*type_string_func)(type_id type);

void cstyle_write_opcode(char *code, size_t *offset, opcode *o, type_string_func type_string, int *indentation);
