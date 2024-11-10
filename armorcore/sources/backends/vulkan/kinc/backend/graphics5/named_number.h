#pragma once

#define KINC_INTERNAL_NAMED_NUMBER_COUNT 32

typedef struct {
	char name[256];
	uint32_t number;
} kinc_internal_named_number;
