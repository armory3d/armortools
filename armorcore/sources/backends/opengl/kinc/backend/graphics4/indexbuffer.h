#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t *converted_data;
	void *data;
	int count;
	unsigned usage;
	int format;
	unsigned buffer_id;
} kinc_g4_index_buffer_impl_t;

#ifdef __cplusplus
}
#endif
