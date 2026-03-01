#pragma once

#include "iron_array.h"

buffer_t *lz4_encode(buffer_t *b);
buffer_t *lz4_decode(buffer_t *b, uint32_t olen);
