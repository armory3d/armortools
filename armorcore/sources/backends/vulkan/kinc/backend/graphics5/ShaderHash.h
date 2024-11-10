#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t hash;
	uint32_t index;
} kinc_internal_hash_index_t;

uint32_t kinc_internal_hash_name(unsigned char *str);

#ifdef __cplusplus
}
#endif
