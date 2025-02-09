#pragma once

#include "types.h"

/*! \file int8x16.h
    \brief Provides 128bit sixteen-element unsigned 8-bit integer SIMD operations which are mapped to equivalent SSE2 or Neon operations.
*/

#if defined(KINC_SSE2)

static inline kinc_uint8x16_t kinc_uint8x16_intrin_load(const uint8_t *values) {
	return _mm_load_si128((const kinc_uint8x16_t *)values);
}

static inline kinc_uint8x16_t kinc_uint8x16_intrin_load_unaligned(const uint8_t *values) {
	return _mm_loadu_si128((const kinc_uint8x16_t *)values);
}

static inline kinc_uint8x16_t kinc_uint8x16_load(const uint8_t values[16]) {
	return _mm_set_epi8(values[15], values[14], values[13], values[12], values[11], values[10], values[9], values[8], values[7], values[6], values[5],
	                    values[4], values[3], values[2], values[1], values[0]);
}

static inline kinc_uint8x16_t kinc_uint8x16_load_all(uint8_t t) {
	return _mm_set1_epi8(t);
}

static inline void kinc_uint8x16_store(uint8_t *destination, kinc_uint8x16_t value) {
	_mm_store_si128((kinc_uint8x16_t *)destination, value);
}

static inline void kinc_uint8x16_store_unaligned(uint8_t *destination, kinc_uint8x16_t value) {
	_mm_storeu_si128((kinc_uint8x16_t *)destination, value);
}

static inline uint8_t kinc_uint8x16_get(kinc_uint8x16_t t, int index) {
	union {
		__m128i value;
		uint8_t elements[16];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

static inline kinc_uint8x16_t kinc_uint8x16_add(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_add_epi8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_sub(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_sub_epi8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_max(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_max_epu8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_min(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_min_epu8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpeq(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_cmpeq_epi8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpge(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_cmpeq_epi8(_mm_max_epu8(a, b), a);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpgt(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_xor_si128(_mm_cmpeq_epi8(_mm_max_epu8(b, a), b), _mm_set1_epi8(-1));
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmple(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_cmpeq_epi8(_mm_max_epu8(b, a), b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmplt(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return kinc_uint8x16_cmpgt(b, a);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpneq(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_andnot_si128(_mm_cmpeq_epi8(a, b), _mm_set1_epi32(0xffffffff));
}

static inline kinc_uint8x16_t kinc_uint8x16_sel(kinc_uint8x16_t a, kinc_uint8x16_t b, kinc_uint8x16_mask_t mask) {
	return _mm_xor_si128(b, _mm_and_si128(mask, _mm_xor_si128(a, b)));
}

static inline kinc_uint8x16_t kinc_uint8x16_or(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_or_si128(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_and(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_and_si128(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_xor(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return _mm_xor_si128(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_not(kinc_uint8x16_t t) {
	return _mm_xor_si128(t, _mm_set1_epi32(0xffffffff));
}

#elif defined(KINC_NEON)

static inline kinc_uint8x16_t kinc_uint8x16_intrin_load(const uint8_t *values) {
	return vld1q_u8(values);
}

static inline kinc_uint8x16_t kinc_uint8x16_intrin_load_unaligned(const uint8_t *values) {
	return kinc_uint8x16_intrin_load(values);
}

static inline kinc_uint8x16_t kinc_uint8x16_load(const uint8_t values[16]) {
	return (kinc_uint8x16_t){values[0], values[1], values[2],  values[3],  values[4],  values[5],  values[6],  values[7],
	                         values[8], values[9], values[10], values[11], values[12], values[13], values[14], values[15]};
}

static inline kinc_uint8x16_t kinc_uint8x16_load_all(uint8_t t) {
	return (kinc_uint8x16_t){t, t, t, t, t, t, t, t, t, t, t, t, t, t, t, t};
}

static inline void kinc_uint8x16_store(uint8_t *destination, kinc_uint8x16_t value) {
	vst1q_u8(destination, value);
}

static inline void kinc_uint8x16_store_unaligned(uint8_t *destination, kinc_uint8x16_t value) {
	kinc_uint8x16_store(destination, value);
}

static inline uint8_t kinc_uint8x16_get(kinc_uint8x16_t t, int index) {
	return t[index];
}

static inline kinc_uint8x16_t kinc_uint8x16_add(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vaddq_u8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_sub(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vsubq_u8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_max(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vmaxq_u8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_min(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vminq_u8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpeq(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vceqq_u8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpge(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vcgeq_u8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpgt(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vcgtq_u8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmple(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vcleq_u8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmplt(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vcltq_u8(a, b);
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpneq(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vmvnq_u8(vceqq_u8(a, b));
}

static inline kinc_uint8x16_t kinc_uint8x16_sel(kinc_uint8x16_t a, kinc_uint8x16_t b, kinc_uint8x16_mask_t mask) {
	return vbslq_u8(mask, a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_or(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vorrq_u8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_and(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return vandq_u8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_xor(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	return veorq_u8(a, b);
}

static inline kinc_uint8x16_t kinc_uint8x16_not(kinc_uint8x16_t t) {
	return vmvnq_u8(t);
}

#else

static inline kinc_uint8x16_t kinc_uint8x16_intrin_load(const uint8_t *values) {
	kinc_uint8x16_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	value.values[4] = values[4];
	value.values[5] = values[5];
	value.values[6] = values[6];
	value.values[7] = values[7];
	value.values[8] = values[8];
	value.values[9] = values[9];
	value.values[10] = values[10];
	value.values[11] = values[11];
	value.values[12] = values[12];
	value.values[13] = values[13];
	value.values[14] = values[14];
	value.values[15] = values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_intrin_load_unaligned(const uint8_t *values) {
	return kinc_uint8x16_intrin_load(values);
}

static inline kinc_uint8x16_t kinc_uint8x16_load(const uint8_t values[16]) {
	kinc_uint8x16_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	value.values[4] = values[4];
	value.values[5] = values[5];
	value.values[6] = values[6];
	value.values[7] = values[7];
	value.values[8] = values[8];
	value.values[9] = values[9];
	value.values[10] = values[10];
	value.values[11] = values[11];
	value.values[12] = values[12];
	value.values[13] = values[13];
	value.values[14] = values[14];
	value.values[15] = values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_load_all(uint8_t t) {
	kinc_uint8x16_t value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	value.values[4] = t;
	value.values[5] = t;
	value.values[6] = t;
	value.values[7] = t;
	value.values[8] = t;
	value.values[9] = t;
	value.values[10] = t;
	value.values[11] = t;
	value.values[12] = t;
	value.values[13] = t;
	value.values[14] = t;
	value.values[15] = t;
	return value;
}

static inline void kinc_uint8x16_store(uint8_t *destination, kinc_uint8x16_t value) {
	destination[0] = value.values[0];
	destination[1] = value.values[1];
	destination[2] = value.values[2];
	destination[3] = value.values[3];
	destination[4] = value.values[4];
	destination[5] = value.values[5];
	destination[6] = value.values[6];
	destination[7] = value.values[7];
	destination[8] = value.values[8];
	destination[9] = value.values[9];
	destination[10] = value.values[10];
	destination[11] = value.values[11];
	destination[12] = value.values[12];
	destination[13] = value.values[13];
	destination[14] = value.values[14];
	destination[15] = value.values[15];
}

static inline void kinc_uint8x16_store_unaligned(uint8_t *destination, kinc_uint8x16_t value) {
	return kinc_uint8x16_store(destination, value);
}

static inline uint8_t kinc_uint8x16_get(kinc_uint8x16_t t, int index) {
	return t.values[index];
}

static inline kinc_uint8x16_t kinc_uint8x16_add(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	value.values[4] = a.values[4] + b.values[4];
	value.values[5] = a.values[5] + b.values[5];
	value.values[6] = a.values[6] + b.values[6];
	value.values[7] = a.values[7] + b.values[7];
	value.values[8] = a.values[8] + b.values[8];
	value.values[9] = a.values[9] + b.values[9];
	value.values[10] = a.values[10] + b.values[10];
	value.values[11] = a.values[11] + b.values[11];
	value.values[12] = a.values[12] + b.values[12];
	value.values[13] = a.values[13] + b.values[13];
	value.values[14] = a.values[14] + b.values[14];
	value.values[15] = a.values[15] + b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_sub(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	value.values[4] = a.values[4] - b.values[4];
	value.values[5] = a.values[5] - b.values[5];
	value.values[6] = a.values[6] - b.values[6];
	value.values[7] = a.values[7] - b.values[7];
	value.values[8] = a.values[8] - b.values[8];
	value.values[9] = a.values[9] - b.values[9];
	value.values[10] = a.values[10] - b.values[10];
	value.values[11] = a.values[11] - b.values[11];
	value.values[12] = a.values[12] - b.values[12];
	value.values[13] = a.values[13] - b.values[13];
	value.values[14] = a.values[14] - b.values[14];
	value.values[15] = a.values[15] - b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_max(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] > b.values[0] ? a.values[0] : b.values[0];
	value.values[1] = a.values[1] > b.values[1] ? a.values[1] : b.values[1];
	value.values[2] = a.values[2] > b.values[2] ? a.values[2] : b.values[2];
	value.values[3] = a.values[3] > b.values[3] ? a.values[3] : b.values[3];
	value.values[4] = a.values[4] > b.values[4] ? a.values[4] : b.values[4];
	value.values[5] = a.values[5] > b.values[5] ? a.values[5] : b.values[5];
	value.values[6] = a.values[6] > b.values[6] ? a.values[6] : b.values[6];
	value.values[7] = a.values[7] > b.values[7] ? a.values[7] : b.values[7];
	value.values[8] = a.values[8] > b.values[8] ? a.values[8] : b.values[8];
	value.values[9] = a.values[9] > b.values[9] ? a.values[9] : b.values[9];
	value.values[10] = a.values[10] > b.values[10] ? a.values[10] : b.values[10];
	value.values[11] = a.values[11] > b.values[11] ? a.values[11] : b.values[11];
	value.values[12] = a.values[12] > b.values[12] ? a.values[12] : b.values[12];
	value.values[13] = a.values[13] > b.values[13] ? a.values[13] : b.values[13];
	value.values[14] = a.values[14] > b.values[14] ? a.values[14] : b.values[14];
	value.values[15] = a.values[15] > b.values[15] ? a.values[15] : b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_min(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] > b.values[0] ? b.values[0] : a.values[0];
	value.values[1] = a.values[1] > b.values[1] ? b.values[1] : a.values[1];
	value.values[2] = a.values[2] > b.values[2] ? b.values[2] : a.values[2];
	value.values[3] = a.values[3] > b.values[3] ? b.values[3] : a.values[3];
	value.values[4] = a.values[4] > b.values[4] ? b.values[4] : a.values[4];
	value.values[5] = a.values[5] > b.values[5] ? b.values[5] : a.values[5];
	value.values[6] = a.values[6] > b.values[6] ? b.values[6] : a.values[6];
	value.values[7] = a.values[7] > b.values[7] ? b.values[7] : a.values[7];
	value.values[8] = a.values[8] > b.values[8] ? b.values[8] : a.values[8];
	value.values[9] = a.values[9] > b.values[9] ? b.values[9] : a.values[9];
	value.values[10] = a.values[10] > b.values[10] ? b.values[10] : a.values[10];
	value.values[11] = a.values[11] > b.values[11] ? b.values[11] : a.values[11];
	value.values[12] = a.values[12] > b.values[12] ? b.values[12] : a.values[12];
	value.values[13] = a.values[13] > b.values[13] ? b.values[13] : a.values[13];
	value.values[14] = a.values[14] > b.values[14] ? b.values[14] : a.values[14];
	value.values[15] = a.values[15] > b.values[15] ? b.values[15] : a.values[15];
	return value;
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpeq(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_mask_t mask;
	mask.values[0] = a.values[0] == b.values[0] ? 0xff : 0;
	mask.values[1] = a.values[1] == b.values[1] ? 0xff : 0;
	mask.values[2] = a.values[2] == b.values[2] ? 0xff : 0;
	mask.values[3] = a.values[3] == b.values[3] ? 0xff : 0;
	mask.values[4] = a.values[4] == b.values[4] ? 0xff : 0;
	mask.values[5] = a.values[5] == b.values[5] ? 0xff : 0;
	mask.values[6] = a.values[6] == b.values[6] ? 0xff : 0;
	mask.values[7] = a.values[7] == b.values[7] ? 0xff : 0;
	mask.values[8] = a.values[8] == b.values[8] ? 0xff : 0;
	mask.values[9] = a.values[9] == b.values[9] ? 0xff : 0;
	mask.values[10] = a.values[10] == b.values[10] ? 0xff : 0;
	mask.values[11] = a.values[11] == b.values[11] ? 0xff : 0;
	mask.values[12] = a.values[12] == b.values[12] ? 0xff : 0;
	mask.values[13] = a.values[13] == b.values[13] ? 0xff : 0;
	mask.values[14] = a.values[14] == b.values[14] ? 0xff : 0;
	mask.values[15] = a.values[15] == b.values[15] ? 0xff : 0;
	return mask;
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpge(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_mask_t mask;
	mask.values[0] = a.values[0] >= b.values[0] ? 0xff : 0;
	mask.values[1] = a.values[1] >= b.values[1] ? 0xff : 0;
	mask.values[2] = a.values[2] >= b.values[2] ? 0xff : 0;
	mask.values[3] = a.values[3] >= b.values[3] ? 0xff : 0;
	mask.values[4] = a.values[4] >= b.values[4] ? 0xff : 0;
	mask.values[5] = a.values[5] >= b.values[5] ? 0xff : 0;
	mask.values[6] = a.values[6] >= b.values[6] ? 0xff : 0;
	mask.values[7] = a.values[7] >= b.values[7] ? 0xff : 0;
	mask.values[8] = a.values[8] >= b.values[8] ? 0xff : 0;
	mask.values[9] = a.values[9] >= b.values[9] ? 0xff : 0;
	mask.values[10] = a.values[10] >= b.values[10] ? 0xff : 0;
	mask.values[11] = a.values[11] >= b.values[11] ? 0xff : 0;
	mask.values[12] = a.values[12] >= b.values[12] ? 0xff : 0;
	mask.values[13] = a.values[13] >= b.values[13] ? 0xff : 0;
	mask.values[14] = a.values[14] >= b.values[14] ? 0xff : 0;
	mask.values[15] = a.values[15] >= b.values[15] ? 0xff : 0;
	return mask;
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpgt(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_mask_t mask;
	mask.values[0] = a.values[0] > b.values[0] ? 0xff : 0;
	mask.values[1] = a.values[1] > b.values[1] ? 0xff : 0;
	mask.values[2] = a.values[2] > b.values[2] ? 0xff : 0;
	mask.values[3] = a.values[3] > b.values[3] ? 0xff : 0;
	mask.values[4] = a.values[4] > b.values[4] ? 0xff : 0;
	mask.values[5] = a.values[5] > b.values[5] ? 0xff : 0;
	mask.values[6] = a.values[6] > b.values[6] ? 0xff : 0;
	mask.values[7] = a.values[7] > b.values[7] ? 0xff : 0;
	mask.values[8] = a.values[8] > b.values[8] ? 0xff : 0;
	mask.values[9] = a.values[9] > b.values[9] ? 0xff : 0;
	mask.values[10] = a.values[10] > b.values[10] ? 0xff : 0;
	mask.values[11] = a.values[11] > b.values[11] ? 0xff : 0;
	mask.values[12] = a.values[12] > b.values[12] ? 0xff : 0;
	mask.values[13] = a.values[13] > b.values[13] ? 0xff : 0;
	mask.values[14] = a.values[14] > b.values[14] ? 0xff : 0;
	mask.values[15] = a.values[15] > b.values[15] ? 0xff : 0;
	return mask;
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmple(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_mask_t mask;
	mask.values[0] = a.values[0] <= b.values[0] ? 0xff : 0;
	mask.values[1] = a.values[1] <= b.values[1] ? 0xff : 0;
	mask.values[2] = a.values[2] <= b.values[2] ? 0xff : 0;
	mask.values[3] = a.values[3] <= b.values[3] ? 0xff : 0;
	mask.values[4] = a.values[4] <= b.values[4] ? 0xff : 0;
	mask.values[5] = a.values[5] <= b.values[5] ? 0xff : 0;
	mask.values[6] = a.values[6] <= b.values[6] ? 0xff : 0;
	mask.values[7] = a.values[7] <= b.values[7] ? 0xff : 0;
	mask.values[8] = a.values[8] <= b.values[8] ? 0xff : 0;
	mask.values[9] = a.values[9] <= b.values[9] ? 0xff : 0;
	mask.values[10] = a.values[10] <= b.values[10] ? 0xff : 0;
	mask.values[11] = a.values[11] <= b.values[11] ? 0xff : 0;
	mask.values[12] = a.values[12] <= b.values[12] ? 0xff : 0;
	mask.values[13] = a.values[13] <= b.values[13] ? 0xff : 0;
	mask.values[14] = a.values[14] <= b.values[14] ? 0xff : 0;
	mask.values[15] = a.values[15] <= b.values[15] ? 0xff : 0;
	return mask;
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmplt(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_mask_t mask;
	mask.values[0] = a.values[0] < b.values[0] ? 0xff : 0;
	mask.values[1] = a.values[1] < b.values[1] ? 0xff : 0;
	mask.values[2] = a.values[2] < b.values[2] ? 0xff : 0;
	mask.values[3] = a.values[3] < b.values[3] ? 0xff : 0;
	mask.values[4] = a.values[4] < b.values[4] ? 0xff : 0;
	mask.values[5] = a.values[5] < b.values[5] ? 0xff : 0;
	mask.values[6] = a.values[6] < b.values[6] ? 0xff : 0;
	mask.values[7] = a.values[7] < b.values[7] ? 0xff : 0;
	mask.values[8] = a.values[8] < b.values[8] ? 0xff : 0;
	mask.values[9] = a.values[9] < b.values[9] ? 0xff : 0;
	mask.values[10] = a.values[10] < b.values[10] ? 0xff : 0;
	mask.values[11] = a.values[11] < b.values[11] ? 0xff : 0;
	mask.values[12] = a.values[12] < b.values[12] ? 0xff : 0;
	mask.values[13] = a.values[13] < b.values[13] ? 0xff : 0;
	mask.values[14] = a.values[14] < b.values[14] ? 0xff : 0;
	mask.values[15] = a.values[15] < b.values[15] ? 0xff : 0;
	return mask;
}

static inline kinc_uint8x16_mask_t kinc_uint8x16_cmpneq(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_mask_t mask;
	mask.values[0] = a.values[0] != b.values[0] ? 0xff : 0;
	mask.values[1] = a.values[1] != b.values[1] ? 0xff : 0;
	mask.values[2] = a.values[2] != b.values[2] ? 0xff : 0;
	mask.values[3] = a.values[3] != b.values[3] ? 0xff : 0;
	mask.values[4] = a.values[4] != b.values[4] ? 0xff : 0;
	mask.values[5] = a.values[5] != b.values[5] ? 0xff : 0;
	mask.values[6] = a.values[6] != b.values[6] ? 0xff : 0;
	mask.values[7] = a.values[7] != b.values[7] ? 0xff : 0;
	mask.values[8] = a.values[8] != b.values[8] ? 0xff : 0;
	mask.values[9] = a.values[9] != b.values[9] ? 0xff : 0;
	mask.values[10] = a.values[10] != b.values[10] ? 0xff : 0;
	mask.values[11] = a.values[11] != b.values[11] ? 0xff : 0;
	mask.values[12] = a.values[12] != b.values[12] ? 0xff : 0;
	mask.values[13] = a.values[13] != b.values[13] ? 0xff : 0;
	mask.values[14] = a.values[14] != b.values[14] ? 0xff : 0;
	mask.values[15] = a.values[15] != b.values[15] ? 0xff : 0;
	return mask;
}

static inline kinc_uint8x16_t kinc_uint8x16_sel(kinc_uint8x16_t a, kinc_uint8x16_t b, kinc_uint8x16_mask_t mask) {
	kinc_uint8x16_t value;
	value.values[0] = mask.values[0] != 0 ? a.values[0] : b.values[0];
	value.values[1] = mask.values[1] != 0 ? a.values[1] : b.values[1];
	value.values[2] = mask.values[2] != 0 ? a.values[2] : b.values[2];
	value.values[3] = mask.values[3] != 0 ? a.values[3] : b.values[3];
	value.values[4] = mask.values[4] != 0 ? a.values[4] : b.values[4];
	value.values[5] = mask.values[5] != 0 ? a.values[5] : b.values[5];
	value.values[6] = mask.values[6] != 0 ? a.values[6] : b.values[6];
	value.values[7] = mask.values[7] != 0 ? a.values[7] : b.values[7];
	value.values[8] = mask.values[8] != 0 ? a.values[8] : b.values[8];
	value.values[9] = mask.values[9] != 0 ? a.values[9] : b.values[9];
	value.values[10] = mask.values[10] != 0 ? a.values[10] : b.values[10];
	value.values[11] = mask.values[11] != 0 ? a.values[11] : b.values[11];
	value.values[12] = mask.values[12] != 0 ? a.values[12] : b.values[12];
	value.values[13] = mask.values[13] != 0 ? a.values[13] : b.values[13];
	value.values[14] = mask.values[14] != 0 ? a.values[14] : b.values[14];
	value.values[15] = mask.values[15] != 0 ? a.values[15] : b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_or(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] | b.values[0];
	value.values[1] = a.values[1] | b.values[1];
	value.values[2] = a.values[2] | b.values[2];
	value.values[3] = a.values[3] | b.values[3];
	value.values[4] = a.values[4] | b.values[4];
	value.values[5] = a.values[5] | b.values[5];
	value.values[6] = a.values[6] | b.values[6];
	value.values[7] = a.values[7] | b.values[7];
	value.values[8] = a.values[8] | b.values[8];
	value.values[9] = a.values[9] | b.values[9];
	value.values[10] = a.values[10] | b.values[10];
	value.values[11] = a.values[11] | b.values[11];
	value.values[12] = a.values[12] | b.values[12];
	value.values[13] = a.values[13] | b.values[13];
	value.values[14] = a.values[14] | b.values[14];
	value.values[15] = a.values[15] | b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_and(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] & b.values[0];
	value.values[1] = a.values[1] & b.values[1];
	value.values[2] = a.values[2] & b.values[2];
	value.values[3] = a.values[3] & b.values[3];
	value.values[4] = a.values[4] & b.values[4];
	value.values[5] = a.values[5] & b.values[5];
	value.values[6] = a.values[6] & b.values[6];
	value.values[7] = a.values[7] & b.values[7];
	value.values[8] = a.values[8] & b.values[8];
	value.values[9] = a.values[9] & b.values[9];
	value.values[10] = a.values[10] & b.values[10];
	value.values[11] = a.values[11] & b.values[11];
	value.values[12] = a.values[12] & b.values[12];
	value.values[13] = a.values[13] & b.values[13];
	value.values[14] = a.values[14] & b.values[14];
	value.values[15] = a.values[15] & b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_xor(kinc_uint8x16_t a, kinc_uint8x16_t b) {
	kinc_uint8x16_t value;
	value.values[0] = a.values[0] ^ b.values[0];
	value.values[1] = a.values[1] ^ b.values[1];
	value.values[2] = a.values[2] ^ b.values[2];
	value.values[3] = a.values[3] ^ b.values[3];
	value.values[4] = a.values[4] ^ b.values[4];
	value.values[5] = a.values[5] ^ b.values[5];
	value.values[6] = a.values[6] ^ b.values[6];
	value.values[7] = a.values[7] ^ b.values[7];
	value.values[8] = a.values[8] ^ b.values[8];
	value.values[9] = a.values[9] ^ b.values[9];
	value.values[10] = a.values[10] ^ b.values[10];
	value.values[11] = a.values[11] ^ b.values[11];
	value.values[12] = a.values[12] ^ b.values[12];
	value.values[13] = a.values[13] ^ b.values[13];
	value.values[14] = a.values[14] ^ b.values[14];
	value.values[15] = a.values[15] ^ b.values[15];
	return value;
}

static inline kinc_uint8x16_t kinc_uint8x16_not(kinc_uint8x16_t t) {
	kinc_uint8x16_t value;
	value.values[0] = ~t.values[0];
	value.values[1] = ~t.values[1];
	value.values[2] = ~t.values[2];
	value.values[3] = ~t.values[3];
	value.values[4] = ~t.values[4];
	value.values[5] = ~t.values[5];
	value.values[6] = ~t.values[6];
	value.values[7] = ~t.values[7];
	value.values[8] = ~t.values[8];
	value.values[9] = ~t.values[9];
	value.values[10] = ~t.values[10];
	value.values[11] = ~t.values[11];
	value.values[12] = ~t.values[12];
	value.values[13] = ~t.values[13];
	value.values[14] = ~t.values[14];
	value.values[15] = ~t.values[15];
	return value;
}

#endif
