#pragma once

#include <stddef.h>
#include <stdint.h>

#define NO_NAME 0

typedef size_t name_id;

void names_init(void);

name_id add_name(char *name);

char *get_name(name_id index);
