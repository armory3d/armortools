#pragma once

#include "types.h"
#include <kinc/global.h>
#include <string.h>

/*! \file type_conversions.h
    \brief Provides type casts and type conversions between all 128bit SIMD types
*/

#if defined(KINC_SSE2)

// Float32x4 ----> Other
static inline kinc_int32x4_t kinc_float32x4_cast_to_int32x4(kinc_float32x4_t t) {
	return _mm_castps_si128(t);
}

static inline kinc_uint32x4_t kinc_float32x4_cast_to_uint32x4(kinc_float32x4_t t) {
	return _mm_castps_si128(t);
}

static inline kinc_int16x8_t kinc_float32x4_cast_to_int16x8(kinc_float32x4_t t) {
	return _mm_castps_si128(t);
}

static inline kinc_uint16x8_t kinc_float32x4_cast_to_uint16x8(kinc_float32x4_t t) {
	return _mm_castps_si128(t);
}

static inline kinc_int8x16_t kinc_float32x4_cast_to_int8x16(kinc_float32x4_t t) {
	return _mm_castps_si128(t);
}

static inline kinc_uint8x16_t kinc_float32x4_cast_to_uint8x16(kinc_float32x4_t t) {
	return _mm_castps_si128(t);
}

// Int32x4 ----> Other
static inline kinc_float32x4_t kinc_int32x4_cast_to_float32x4(kinc_int32x4_t t) {
	return _mm_castsi128_ps(t);
}

static inline kinc_uint32x4_t kinc_int32x4_cast_to_uint32x4(kinc_int32x4_t t) {
	// SSE2's m128i is every int type, so we can just return any inbound int type parameter
	return t;
}

static inline kinc_int16x8_t kinc_int32x4_cast_to_int16x8(kinc_int32x4_t t) {
	return t;
}

static inline kinc_uint16x8_t kinc_int32x4_cast_to_uint16x8(kinc_int32x4_t t) {
	return t;
}

static inline kinc_int8x16_t kinc_int32x4_cast_to_int8x16(kinc_int32x4_t t) {
	return t;
}

static inline kinc_uint8x16_t kinc_int32x4_cast_to_uint8x16(kinc_int32x4_t t) {
	return t;
}

// Unsigned Int32x4 ----> Other
static inline kinc_float32x4_t kinc_uint32x4_cast_to_float32x4(kinc_uint32x4_t t) {
	return _mm_castsi128_ps(t);
}

static inline kinc_int32x4_t kinc_uint32x4_cast_to_int32x4(kinc_uint32x4_t t) {
	return t;
}

static inline kinc_int16x8_t kinc_uint32x4_cast_to_int16x8(kinc_uint32x4_t t) {
	return t;
}

static inline kinc_uint16x8_t kinc_uint32x4_cast_to_uint16x8(kinc_uint32x4_t t) {
	return t;
}

static inline kinc_int8x16_t kinc_uint32x4_cast_to_int8x16(kinc_uint32x4_t t) {
	return t;
}

static inline kinc_uint8x16_t kinc_uint32x4_cast_to_uint8x16(kinc_uint32x4_t t) {
	return t;
}

// Int16x8 ----> Other
static inline kinc_float32x4_t kinc_int16x8_cast_to_float32x4(kinc_int16x8_t t) {
	return _mm_castsi128_ps(t);
}

static inline kinc_int32x4_t kinc_int16x8_cast_to_int32x4(kinc_int16x8_t t) {
	return t;
}

static inline kinc_uint32x4_t kinc_int16x8_cast_to_uint32x4(kinc_int16x8_t t) {
	return t;
}

static inline kinc_uint16x8_t kinc_int16x8_cast_to_uint16x8(kinc_int16x8_t t) {
	return t;
}

static inline kinc_int8x16_t kinc_int16x8_cast_to_int8x16(kinc_int16x8_t t) {
	return t;
}

static inline kinc_uint8x16_t kinc_int16x8_cast_to_uint8x16(kinc_int16x8_t t) {
	return t;
}

// Unsigned Int16x8 ----> Other
static inline kinc_float32x4_t kinc_uint16x8_cast_to_float32x4(kinc_uint16x8_t t) {
	return _mm_castsi128_ps(t);
}

static inline kinc_int32x4_t kinc_uint16x8_cast_to_int32x4(kinc_uint16x8_t t) {
	return t;
}

static inline kinc_uint32x4_t kinc_uint16x8_cast_to_uint32x4(kinc_uint16x8_t t) {
	return t;
}

static inline kinc_int16x8_t kinc_uint16x8_cast_to_int16x8(kinc_uint16x8_t t) {
	return t;
}

static inline kinc_int8x16_t kinc_uint16x8_cast_to_int8x16(kinc_uint16x8_t t) {
	return t;
}

static inline kinc_uint8x16_t kinc_uint16x8_cast_to_uint8x16(kinc_uint16x8_t t) {
	return t;
}

// Int8x16 ----> Other
static inline kinc_float32x4_t kinc_int8x16_cast_to_float32x4(kinc_int8x16_t t) {
	return _mm_castsi128_ps(t);
}

static inline kinc_int32x4_t kinc_int8x16_cast_to_int32x4(kinc_int8x16_t t) {
	return t;
}

static inline kinc_uint32x4_t kinc_int8x16_cast_to_uint32x4(kinc_int8x16_t t) {
	return t;
}

static inline kinc_int16x8_t kinc_int8x16_cast_to_int16x8(kinc_int8x16_t t) {
	return t;
}

static inline kinc_uint16x8_t kinc_int8x16_cast_to_uint16x8(kinc_int8x16_t t) {
	return t;
}

static inline kinc_uint8x16_t kinc_int8x16_cast_to_uint8x16(kinc_int8x16_t t) {
	return t;
}

// Unsigned Int8x16 ----> Other
static inline kinc_float32x4_t kinc_uint8x16_cast_to_float32x4(kinc_uint8x16_t t) {
	return _mm_castsi128_ps(t);
}

static inline kinc_int32x4_t kinc_uint8x16_cast_to_int32x4(kinc_uint8x16_t t) {
	return t;
}

static inline kinc_uint32x4_t kinc_uint8x16_cast_to_uint32x4(kinc_uint8x16_t t) {
	return t;
}

static inline kinc_int16x8_t kinc_uint8x16_cast_to_int16x8(kinc_uint8x16_t t) {
	return t;
}

static inline kinc_uint16x8_t kinc_uint8x16_cast_to_uint16x8(kinc_uint8x16_t t) {
	return t;
}

static inline kinc_int8x16_t kinc_uint8x16_cast_to_int8x16(kinc_uint8x16_t t) {
	return t;
}

#elif defined(KINC_SSE)

// Float32x4 ----> Other
static inline kinc_int32x4_t kinc_float32x4_cast_to_int32x4(kinc_float32x4_t t) {
	float extracted[4];
	_mm_storeu_ps(&extracted[0], t);

	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &extracted[0], sizeof(extracted));

	return cvt;
}

static inline kinc_uint32x4_t kinc_float32x4_cast_to_uint32x4(kinc_float32x4_t t) {
	float extracted[4];
	_mm_storeu_ps(&extracted[0], t);

	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &extracted[0], sizeof(extracted));

	return cvt;
}

static inline kinc_int16x8_t kinc_float32x4_cast_to_int16x8(kinc_float32x4_t t) {
	float extracted[4];
	_mm_storeu_ps(&extracted[0], t);

	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &extracted[0], sizeof(extracted));

	return cvt;
}

static inline kinc_uint16x8_t kinc_float32x4_cast_to_uint16x8(kinc_float32x4_t t) {
	float extracted[4];
	_mm_storeu_ps(&extracted[0], t);

	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &extracted[0], sizeof(extracted));

	return cvt;
}

static inline kinc_int8x16_t kinc_float32x4_cast_to_int8x16(kinc_float32x4_t t) {
	float extracted[4];
	_mm_storeu_ps(&extracted[0], t);

	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &extracted[0], sizeof(extracted));

	return cvt;
}

static inline kinc_uint8x16_t kinc_float32x4_cast_to_uint8x16(kinc_float32x4_t t) {
	float extracted[4];
	_mm_storeu_ps(&extracted[0], t);

	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &extracted[0], sizeof(extracted));

	return cvt;
}

// Int32x4 ----> Other
static inline kinc_float32x4_t kinc_int32x4_cast_to_float32x4(kinc_int32x4_t t) {
	float cvt[4];
	memcpy(&cvt[0], &t.values[0], sizeof(t));

	return _mm_loadu_ps(&cvt[0]);
}

// Unsigned Int32x4 ----> Other
static inline kinc_float32x4_t kinc_uint32x4_cast_to_float32x4(kinc_uint32x4_t t) {
	float cvt[4];
	memcpy(&cvt[0], &t.values[0], sizeof(t));

	return _mm_loadu_ps(&cvt[0]);
}

// Int16x8 ----> Other
static inline kinc_float32x4_t kinc_int16x8_cast_to_float32x4(kinc_int16x8_t t) {
	float cvt[4];
	memcpy(&cvt[0], &t.values[0], sizeof(t));

	return _mm_loadu_ps(&cvt[0]);
}

// Unsigned Int16x8 ----> Other
static inline kinc_float32x4_t kinc_uint16x8_cast_to_float32x4(kinc_uint16x8_t t) {
	float cvt[4];
	memcpy(&cvt[0], &t.values[0], sizeof(t));

	return _mm_loadu_ps(&cvt[0]);
}

// Int8x16 ----> Other
static inline kinc_float32x4_t kinc_int8x16_cast_to_float32x4(kinc_int8x16_t t) {
	float cvt[4];
	memcpy(&cvt[0], &t.values[0], sizeof(t));

	return _mm_loadu_ps(&cvt[0]);
}

// Unsigned Int8x16 ----> Other
static inline kinc_float32x4_t kinc_uint8x16_cast_to_float32x4(kinc_uint8x16_t t) {
	float cvt[4];
	memcpy(&cvt[0], &t.values[0], sizeof(t));

	return _mm_loadu_ps(&cvt[0]);
}

#elif defined(KINC_NEON)

// Float32x4 ----> Other
static inline kinc_int32x4_t kinc_float32x4_cast_to_int32x4(kinc_float32x4_t t) {
	return vreinterpretq_s32_f32(t);
}

static inline kinc_uint32x4_t kinc_float32x4_cast_to_uint32x4(kinc_float32x4_t t) {
	return vreinterpretq_u32_f32(t);
}

static inline kinc_int16x8_t kinc_float32x4_cast_to_int16x8(kinc_float32x4_t t) {
	return vreinterpretq_s16_f32(t);
}

static inline kinc_uint16x8_t kinc_float32x4_cast_to_uint16x8(kinc_float32x4_t t) {
	return vreinterpretq_u16_f32(t);
}

static inline kinc_int8x16_t kinc_float32x4_cast_to_int8x16(kinc_float32x4_t t) {
	return vreinterpretq_s8_f32(t);
}

static inline kinc_uint8x16_t kinc_float32x4_cast_to_uint8x16(kinc_float32x4_t t) {
	return vreinterpretq_u8_f32(t);
}

// Int32x4 ----> Other
static inline kinc_float32x4_t kinc_int32x4_cast_to_float32x4(kinc_int32x4_t t) {
	return vreinterpretq_f32_s32(t);
}

static inline kinc_uint32x4_t kinc_int32x4_cast_to_uint32x4(kinc_int32x4_t t) {
	return vreinterpretq_u32_s32(t);
}

static inline kinc_int16x8_t kinc_int32x4_cast_to_int16x8(kinc_int32x4_t t) {
	return vreinterpretq_s16_s32(t);
}

static inline kinc_uint16x8_t kinc_int32x4_cast_to_uint16x8(kinc_int32x4_t t) {
	return vreinterpretq_u16_s32(t);
}

static inline kinc_int8x16_t kinc_int32x4_cast_to_int8x16(kinc_int32x4_t t) {
	return vreinterpretq_s8_s32(t);
}

static inline kinc_uint8x16_t kinc_int32x4_cast_to_uint8x16(kinc_int32x4_t t) {
	return vreinterpretq_u8_s32(t);
}

// Unsigned Int32x4 ----> Other
static inline kinc_float32x4_t kinc_uint32x4_cast_to_float32x4(kinc_uint32x4_t t) {
	return vreinterpretq_f32_u32(t);
}

static inline kinc_int32x4_t kinc_uint32x4_cast_to_int32x4(kinc_uint32x4_t t) {
	return vreinterpretq_s32_u32(t);
}

static inline kinc_int16x8_t kinc_uint32x4_cast_to_int16x8(kinc_uint32x4_t t) {
	return vreinterpretq_s16_u32(t);
}

static inline kinc_uint16x8_t kinc_uint32x4_cast_to_uint16x8(kinc_uint32x4_t t) {
	return vreinterpretq_u16_u32(t);
}

static inline kinc_int8x16_t kinc_uint32x4_cast_to_int8x16(kinc_uint32x4_t t) {
	return vreinterpretq_s8_u32(t);
}

static inline kinc_uint8x16_t kinc_uint32x4_cast_to_uint8x16(kinc_uint32x4_t t) {
	return vreinterpretq_u8_u32(t);
}

// Int16x8 ----> Other
static inline kinc_float32x4_t kinc_int16x8_cast_to_float32x4(kinc_int16x8_t t) {
	return vreinterpretq_f32_s16(t);
}

static inline kinc_int32x4_t kinc_int16x8_cast_to_int32x4(kinc_int16x8_t t) {
	return vreinterpretq_s32_s16(t);
}

static inline kinc_uint32x4_t kinc_int16x8_cast_to_uint32x4(kinc_int16x8_t t) {
	return vreinterpretq_u32_s16(t);
}

static inline kinc_uint16x8_t kinc_int16x8_cast_to_uint16x8(kinc_int16x8_t t) {
	return vreinterpretq_u16_s16(t);
}

static inline kinc_int8x16_t kinc_int16x8_cast_to_int8x16(kinc_int16x8_t t) {
	return vreinterpretq_s8_s16(t);
}

static inline kinc_uint8x16_t kinc_int16x8_cast_to_uint8x16(kinc_int16x8_t t) {
	return vreinterpretq_u8_s16(t);
}

// Unsigned Int16x8 ----> Other
static inline kinc_float32x4_t kinc_uint16x8_cast_to_float32x4(kinc_uint16x8_t t) {
	return vreinterpretq_f32_u16(t);
}

static inline kinc_int32x4_t kinc_uint16x8_cast_to_int32x4(kinc_uint16x8_t t) {
	return vreinterpretq_s32_u16(t);
}

static inline kinc_uint32x4_t kinc_uint16x8_cast_to_uint32x4(kinc_uint16x8_t t) {
	return vreinterpretq_u32_u16(t);
}

static inline kinc_int16x8_t kinc_uint16x8_cast_to_int16x8(kinc_uint16x8_t t) {
	return vreinterpretq_s16_u16(t);
}

static inline kinc_int8x16_t kinc_uint16x8_cast_to_int8x16(kinc_uint16x8_t t) {
	return vreinterpretq_s8_u16(t);
}

static inline kinc_uint8x16_t kinc_uint16x8_cast_to_uint8x16(kinc_uint16x8_t t) {
	return vreinterpretq_u8_u16(t);
}

// Int8x16 ----> Other
static inline kinc_float32x4_t kinc_int8x16_cast_to_float32x4(kinc_int8x16_t t) {
	return vreinterpretq_f32_s8(t);
}

static inline kinc_int32x4_t kinc_int8x16_cast_to_int32x4(kinc_int8x16_t t) {
	return vreinterpretq_s32_s8(t);
}

static inline kinc_uint32x4_t kinc_int8x16_cast_to_uint32x4(kinc_int8x16_t t) {
	return vreinterpretq_u32_s8(t);
}

static inline kinc_int16x8_t kinc_int8x16_cast_to_int16x8(kinc_int8x16_t t) {
	return vreinterpretq_s16_s8(t);
}

static inline kinc_uint16x8_t kinc_int8x16_cast_to_uint16x8(kinc_int8x16_t t) {
	return vreinterpretq_u16_s8(t);
}

static inline kinc_uint8x16_t kinc_int8x16_cast_to_uint8x16(kinc_int8x16_t t) {
	return vreinterpretq_u8_s8(t);
}

// Unsigned Int8x16 ----> Other
static inline kinc_float32x4_t kinc_uint8x16_cast_to_float32x4(kinc_uint8x16_t t) {
	return vreinterpretq_f32_u8(t);
}

static inline kinc_int32x4_t kinc_uint8x16_cast_to_int32x4(kinc_uint8x16_t t) {
	return vreinterpretq_s32_u8(t);
}

static inline kinc_uint32x4_t kinc_uint8x16_cast_to_uint32x4(kinc_uint8x16_t t) {
	return vreinterpretq_u32_u8(t);
}

static inline kinc_int16x8_t kinc_uint8x16_cast_to_int16x8(kinc_uint8x16_t t) {
	return vreinterpretq_s16_u8(t);
}

static inline kinc_uint16x8_t kinc_uint8x16_cast_to_uint16x8(kinc_uint8x16_t t) {
	return vreinterpretq_u16_u8(t);
}

static inline kinc_int8x16_t kinc_uint8x16_cast_to_int8x16(kinc_uint8x16_t t) {
	return vreinterpretq_s8_u8(t);
}

// KINC_NOSIMD float fallbacks casts
#else

// Float32x4 ----> Other
static inline kinc_int32x4_t kinc_float32x4_cast_to_int32x4(kinc_float32x4_t t) {
	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint32x4_t kinc_float32x4_cast_to_uint32x4(kinc_float32x4_t t) {
	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int16x8_t kinc_float32x4_cast_to_int16x8(kinc_float32x4_t t) {
	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint16x8_t kinc_float32x4_cast_to_uint16x8(kinc_float32x4_t t) {
	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int8x16_t kinc_float32x4_cast_to_int8x16(kinc_float32x4_t t) {
	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint8x16_t kinc_float32x4_cast_to_uint8x16(kinc_float32x4_t t) {
	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Int32x4 ----> Float32x4
static inline kinc_float32x4_t kinc_int32x4_cast_to_float32x4(kinc_int32x4_t t) {
	kinc_float32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Unsigned Int32x4 ----> Float32x4
static inline kinc_float32x4_t kinc_uint32x4_cast_to_float32x4(kinc_uint32x4_t t) {
	kinc_float32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Int16x8 ----> Float32x4
static inline kinc_float32x4_t kinc_int16x8_cast_to_float32x4(kinc_int16x8_t t) {
	kinc_float32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Unsigned Int16x8 ----> Float32x4
static inline kinc_float32x4_t kinc_uint16x8_cast_to_float32x4(kinc_uint16x8_t t) {
	kinc_float32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Int8x16 ----> Float32x4
static inline kinc_float32x4_t kinc_int8x16_cast_to_float32x4(kinc_int8x16_t t) {
	kinc_float32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Unsigned Int8x16 ----> Float32x4
static inline kinc_float32x4_t kinc_uint8x16_cast_to_float32x4(kinc_uint8x16_t t) {
	kinc_float32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

#endif // KINC_NOSIMD floats

// Shared signed and unsigned integer vectors for SSE and SIMD-fallback
#if !defined(KINC_SSE2) && (defined(KINC_SSE) || defined(KINC_NOSIMD))

// Int32x4 ----> Other
static inline kinc_uint32x4_t kinc_int32x4_cast_to_uint32x4(kinc_int32x4_t t) {
	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int16x8_t kinc_int32x4_cast_to_int16x8(kinc_int32x4_t t) {
	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint16x8_t kinc_int32x4_cast_to_uint16x8(kinc_int32x4_t t) {
	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int8x16_t kinc_int32x4_cast_to_int8x16(kinc_int32x4_t t) {
	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint8x16_t kinc_int32x4_cast_to_uint8x16(kinc_int32x4_t t) {
	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Unsigned Int32x4 ----> Other
static inline kinc_int32x4_t kinc_uint32x4_cast_to_int32x4(kinc_uint32x4_t t) {
	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int16x8_t kinc_uint32x4_cast_to_int16x8(kinc_uint32x4_t t) {
	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint16x8_t kinc_uint32x4_cast_to_uint16x8(kinc_uint32x4_t t) {
	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int8x16_t kinc_uint32x4_cast_to_int8x16(kinc_uint32x4_t t) {
	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint8x16_t kinc_uint32x4_cast_to_uint8x16(kinc_uint32x4_t t) {
	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Int16x8 ----> Other
static inline kinc_int32x4_t kinc_int16x8_cast_to_int32x4(kinc_int16x8_t t) {
	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint32x4_t kinc_int16x8_cast_to_uint32x4(kinc_int16x8_t t) {
	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint16x8_t kinc_int16x8_cast_to_uint16x8(kinc_int16x8_t t) {
	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int8x16_t kinc_int16x8_cast_to_int8x16(kinc_int16x8_t t) {
	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint8x16_t kinc_int16x8_cast_to_uint8x16(kinc_int16x8_t t) {
	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Unsigned Int16x8 ----> Other
static inline kinc_int32x4_t kinc_uint16x8_cast_to_int32x4(kinc_uint16x8_t t) {
	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint32x4_t kinc_uint16x8_cast_to_uint32x4(kinc_uint16x8_t t) {
	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int16x8_t kinc_uint16x8_cast_to_int16x8(kinc_uint16x8_t t) {
	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int8x16_t kinc_uint16x8_cast_to_int8x16(kinc_uint16x8_t t) {
	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint8x16_t kinc_uint16x8_cast_to_uint8x16(kinc_uint16x8_t t) {
	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Int8x16 ----> Other
static inline kinc_int32x4_t kinc_int8x16_cast_to_int32x4(kinc_int8x16_t t) {
	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint32x4_t kinc_int8x16_cast_to_uint32x4(kinc_int8x16_t t) {
	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int16x8_t kinc_int8x16_cast_to_int16x8(kinc_int8x16_t t) {
	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint16x8_t kinc_int8x16_cast_to_uint16x8(kinc_int8x16_t t) {
	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint8x16_t kinc_int8x16_cast_to_uint8x16(kinc_int8x16_t t) {
	kinc_uint8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

// Unsigned Int8x16 ----> Other
static inline kinc_int32x4_t kinc_uint8x16_cast_to_int32x4(kinc_uint8x16_t t) {
	kinc_int32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint32x4_t kinc_uint8x16_cast_to_uint32x4(kinc_uint8x16_t t) {
	kinc_uint32x4_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int16x8_t kinc_uint8x16_cast_to_int16x8(kinc_uint8x16_t t) {
	kinc_int16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_uint16x8_t kinc_uint8x16_cast_to_uint16x8(kinc_uint8x16_t t) {
	kinc_uint16x8_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

static inline kinc_int8x16_t kinc_uint8x16_cast_to_int8x16(kinc_uint8x16_t t) {
	kinc_int8x16_t cvt;
	memcpy(&cvt.values[0], &t.values[0], sizeof(t));

	return cvt;
}

#endif // KINC_SSE || KINC_NOSIMD
