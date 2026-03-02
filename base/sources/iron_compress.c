
#ifdef WITH_COMPRESS

#include "iron_compress.h"

#define SDEFL_IMPLEMENTATION
#include "sdefl.h"
#define SINFL_IMPLEMENTATION
#include "sinfl.h"

buffer_t *iron_inflate(buffer_t *bytes, bool raw) {
	unsigned char *inflated     = NULL;
	int            inflated_len = bytes->length * 2;
	int            out_len;
	while (1) {
		inflated = (unsigned char *)realloc(inflated, inflated_len);
		out_len  = raw ? sinflate(inflated, inflated_len, bytes->buffer, bytes->length) : zsinflate(inflated, inflated_len, bytes->buffer, bytes->length);
		if (out_len >= 0 && out_len < inflated_len) {
			break;
		}
		inflated_len *= 2;
	}
	buffer_t *output = buffer_create(0);
	output->buffer   = inflated;
	output->length   = out_len;
	return output;
}

buffer_t *iron_deflate(buffer_t *bytes, bool raw) {
	struct sdefl sdefl;
	memset(&sdefl, 0, sizeof(sdefl));
	void     *deflated = malloc(sdefl_bound(bytes->length));
	int       out_len  = raw ? sdeflate(&sdefl, deflated, bytes->buffer, bytes->length, SDEFL_LVL_MIN)
	                         : zsdeflate(&sdefl, deflated, bytes->buffer, bytes->length, SDEFL_LVL_MIN);
	buffer_t *output   = buffer_create(0);
	output->buffer     = deflated;
	output->length     = out_len;
	return output;
}

unsigned char *iron_deflate_raw(unsigned char *data, int data_len, int *out_len, int quality) {
	struct sdefl sdefl;
	memset(&sdefl, 0, sizeof(sdefl));
	void *deflated = malloc(sdefl_bound(data_len));
	*out_len       = zsdeflate(&sdefl, deflated, data, data_len, SDEFL_LVL_MIN);
	return (unsigned char *)deflated;
}

#endif
