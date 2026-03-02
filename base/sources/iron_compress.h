#pragma once

#include "iron_array.h"
#include <stdbool.h>

buffer_t *iron_inflate(buffer_t *bytes, bool raw);
buffer_t *iron_deflate(buffer_t *bytes, bool raw);
unsigned char *iron_deflate_raw(unsigned char *data, int data_len, int *out_len, int quality);
