#pragma once

#include "types.h"

/*! \file int32x4.h
    \brief Provides 128bit four-element signed 32-bit integer SIMD operations which are mapped to equivalent SSE2 or Neon operations.
*/

#if defined(KINC_SSE2)

static inline kinc_int32x4_t kinc_int32x4_intrin_load(const int32_t *values) {
	return _mm_load_si128((const kinc_int32x4_t *)values);
}

static inline kinc_int32x4_t kinc_int32x4_intrin_load_unaligned(const int32_t *values) {
	return _mm_loadu_si128((const kinc_int32x4_t *)values);
}

static inline kinc_int32x4_t kinc_int32x4_load(const int32_t values[4]) {
	return _mm_set_epi32(values[3], values[2], values[1], values[0]);
}

static inline kinc_int32x4_t kinc_int32x4_load_all(int32_t t) {
	return _mm_set1_epi32(t);
}

static inline void kinc_int32x4_store(int32_t *destination, kinc_int32x4_t value) {
	_mm_store_si128((kinc_int32x4_t *)destination, value);
}

static inline void kinc_int32x4_store_unaligned(int32_t *destination, kinc_int32x4_t value) {
	_mm_storeu_si128((kinc_int32x4_t *)destination, value);
}

static inline int32_t kinc_int32x4_get(kinc_int32x4_t t, int index) {
	union {
		__m128i value;
		int32_t elements[4];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

static inline kinc_int32x4_t kinc_int32x4_add(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_add_epi32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_sub(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_sub_epi32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_max(kinc_int32x4_t a, kinc_int32x4_t b) {
	__m128i mask = _mm_cmpgt_epi32(a, b);
	return _mm_xor_si128(b, _mm_and_si128(mask, _mm_xor_si128(a, b)));
}

static inline kinc_int32x4_t kinc_int32x4_min(kinc_int32x4_t a, kinc_int32x4_t b) {
	__m128i mask = _mm_cmplt_epi32(a, b);
	return _mm_xor_si128(b, _mm_and_si128(mask, _mm_xor_si128(a, b)));
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpeq(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_cmpeq_epi32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpge(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_or_si128(_mm_cmpgt_epi32(a, b), _mm_cmpeq_epi32(a, b));
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpgt(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_cmpgt_epi32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmple(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_or_si128(_mm_cmplt_epi32(a, b), _mm_cmpeq_epi32(a, b));
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmplt(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_cmplt_epi32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpneq(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_andnot_si128(_mm_cmpeq_epi32(a, b), _mm_set1_epi32(0xffffffff));
}

static inline kinc_int32x4_t kinc_int32x4_sel(kinc_int32x4_t a, kinc_int32x4_t b, kinc_int32x4_mask_t mask) {
	return _mm_xor_si128(b, _mm_and_si128(mask, _mm_xor_si128(a, b)));
}

static inline kinc_int32x4_t kinc_int32x4_or(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_or_si128(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_and(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_and_si128(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_xor(kinc_int32x4_t a, kinc_int32x4_t b) {
	return _mm_xor_si128(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_not(kinc_int32x4_t t) {
	return _mm_xor_si128(t, _mm_set1_epi32(0xffffffff));
}

#elif defined(KINC_NEON)

static inline kinc_int32x4_t kinc_int32x4_intrin_load(const int32_t *values) {
	return vld1q_s32(values);
}

static inline kinc_int32x4_t kinc_int32x4_intrin_load_unaligned(const int32_t *values) {
	return kinc_int32x4_intrin_load(values);
}

static inline kinc_int32x4_t kinc_int32x4_load(const int32_t values[4]) {
	return (kinc_int32x4_t){values[0], values[1], values[2], values[3]};
}

static inline kinc_int32x4_t kinc_int32x4_load_all(int32_t t) {
	return (kinc_int32x4_t){t, t, t, t};
}

static inline void kinc_int32x4_store(int32_t *destination, kinc_int32x4_t value) {
	vst1q_s32(destination, value);
}

static inline void kinc_int32x4_store_unaligned(int32_t *destination, kinc_int32x4_t value) {
	kinc_int32x4_store(destination, value);
}

static inline int32_t kinc_int32x4_get(kinc_int32x4_t t, int index) {
	return t[index];
}

static inline kinc_int32x4_t kinc_int32x4_add(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vaddq_s32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_sub(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vsubq_s32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_max(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vmaxq_s32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_min(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vminq_s32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpeq(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vceqq_s32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpge(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vcgeq_s32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpgt(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vcgtq_s32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmple(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vcleq_s32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmplt(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vcltq_s32(a, b);
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpneq(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vmvnq_u32(vceqq_s32(a, b));
}

static inline kinc_int32x4_t kinc_int32x4_sel(kinc_int32x4_t a, kinc_int32x4_t b, kinc_int32x4_mask_t mask) {
	return vbslq_s32(mask, a, b);
}

static inline kinc_int32x4_t kinc_int32x4_or(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vorrq_s32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_and(kinc_int32x4_t a, kinc_int32x4_t b) {
	return vandq_s32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_xor(kinc_int32x4_t a, kinc_int32x4_t b) {
	return veorq_s32(a, b);
}

static inline kinc_int32x4_t kinc_int32x4_not(kinc_int32x4_t t) {
	return vmvnq_s32(t);
}

#else

static inline kinc_int32x4_t kinc_int32x4_intrin_load(const int32_t *values) {
	kinc_int32x4_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_intrin_load_unaligned(const int32_t *values) {
	return kinc_int32x4_intrin_load(values);
}

static inline kinc_int32x4_t kinc_int32x4_load(const int32_t values[4]) {
	kinc_int32x4_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_load_all(int32_t t) {
	kinc_int32x4_t value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	return value;
}

static inline void kinc_int32x4_store(int32_t *destination, kinc_int32x4_t value) {
	destination[0] = value.values[0];
	destination[1] = value.values[1];
	destination[2] = value.values[2];
	destination[3] = value.values[3];
}

static inline void kinc_int32x4_store_unaligned(int32_t *destination, kinc_int32x4_t value) {
	kinc_int32x4_store(destination, value);
}

static inline int32_t kinc_int32x4_get(kinc_int32x4_t t, int index) {
	return t.values[index];
}

static inline kinc_int32x4_t kinc_int32x4_add(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_sub(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_max(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] > b.values[0] ? a.values[0] : b.values[0];
	value.values[1] = a.values[1] > b.values[1] ? a.values[1] : b.values[1];
	value.values[2] = a.values[2] > b.values[2] ? a.values[2] : b.values[2];
	value.values[3] = a.values[3] > b.values[3] ? a.values[3] : b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_min(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] > b.values[0] ? b.values[0] : a.values[0];
	value.values[1] = a.values[1] > b.values[1] ? b.values[1] : a.values[1];
	value.values[2] = a.values[2] > b.values[2] ? b.values[2] : a.values[2];
	value.values[3] = a.values[3] > b.values[3] ? b.values[3] : a.values[3];
	return value;
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpeq(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_mask_t mask;
	mask.values[0] = a.values[0] == b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] == b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] == b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] == b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpge(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_mask_t mask;
	mask.values[0] = a.values[0] >= b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] >= b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] >= b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] >= b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpgt(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_mask_t mask;
	mask.values[0] = a.values[0] > b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] > b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] > b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] > b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmple(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_mask_t mask;
	mask.values[0] = a.values[0] <= b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] <= b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] <= b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] <= b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmplt(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_mask_t mask;
	mask.values[0] = a.values[0] < b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] < b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] < b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] < b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_int32x4_mask_t kinc_int32x4_cmpneq(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_mask_t mask;
	mask.values[0] = a.values[0] != b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] != b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] != b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] != b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_int32x4_t kinc_int32x4_sel(kinc_int32x4_t a, kinc_int32x4_t b, kinc_int32x4_mask_t mask) {
	kinc_int32x4_t value;
	value.values[0] = mask.values[0] != 0 ? a.values[0] : b.values[0];
	value.values[1] = mask.values[1] != 0 ? a.values[1] : b.values[1];
	value.values[2] = mask.values[2] != 0 ? a.values[2] : b.values[2];
	value.values[3] = mask.values[3] != 0 ? a.values[3] : b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_or(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] | b.values[0];
	value.values[1] = a.values[1] | b.values[1];
	value.values[2] = a.values[2] | b.values[2];
	value.values[3] = a.values[3] | b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_and(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] & b.values[0];
	value.values[1] = a.values[1] & b.values[1];
	value.values[2] = a.values[2] & b.values[2];
	value.values[3] = a.values[3] & b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_xor(kinc_int32x4_t a, kinc_int32x4_t b) {
	kinc_int32x4_t value;
	value.values[0] = a.values[0] ^ b.values[0];
	value.values[1] = a.values[1] ^ b.values[1];
	value.values[2] = a.values[2] ^ b.values[2];
	value.values[3] = a.values[3] ^ b.values[3];
	return value;
}

static inline kinc_int32x4_t kinc_int32x4_not(kinc_int32x4_t t) {
	kinc_int32x4_t value;
	value.values[0] = ~t.values[0];
	value.values[1] = ~t.values[1];
	value.values[2] = ~t.values[2];
	value.values[3] = ~t.values[3];
	return value;
}

#endif
