#pragma once

#include <vulkan/vulkan_core.h>

#define KINC_INTERNAL_NAMED_NUMBER_COUNT 32

typedef struct {
	char name[256];
	uint32_t number;
} kinc_internal_named_number;

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t hash;
	uint32_t index;
} kinc_internal_hash_index_t;

uint32_t kinc_internal_hash_name(unsigned char *str);
