#pragma once

#include <kinc/global.h>

/*! \file types.h
    \brief Provides 128bit SIMD types which are mapped to equivalent SSE or Neon types.
*/

// Any level of AVX Capability (Could be AVX, AVX2, AVX512, etc.)
//(Currently) only used for checking existence of earlier SSE instruction sets
#if defined(__AVX__)
// Unfortunate situation here
// MSVC does not provide compiletime macros for the following instruction sets
// but their existence is implied by AVX and higher
#define KINC_SSE4_2
#define KINC_SSE4_1
#define KINC_SSSE3
#define KINC_SSE3
#endif

// SSE2 Capability check
// Note for Windows:
//	_M_IX86_FP checks SSE2 and SSE for 32bit Windows programs only, and is unset if not a 32bit program.
//	SSE2 and earlier is --guaranteed-- to be active for any 64bit Windows program
#if defined(__SSE2__) || (_M_IX86_FP == 2) || (defined(KINC_WINDOWS) && defined(KINC_64))
#define KINC_SSE2
#endif

// SSE Capability check
#if defined(__SSE__) || _M_IX86_FP == 2 || _M_IX86_FP == 1 || (defined(KINC_WINDOWS) && !defined(__aarch64__)) ||                                              \
    (defined(KINC_WINDOWSAPP) && !defined(__aarch64__)) || (defined(KINC_MACOS) && __x86_64)

#define KINC_SSE
#endif

// NEON Capability check
#if defined(KINC_IOS) || defined(__aarch64__) || defined(KINC_NEON)
#define KINC_NEON
#endif

// No SIMD Capabilities
#if !defined(KINC_SSE4_2) && !defined(KINC_SSE4_1) && !defined(KINC_SSSE3) && !defined(KINC_SSE3) && !defined(KINC_SSE2) && !defined(KINC_SSE) &&              \
    !defined(KINC_NEON)

#define KINC_NOSIMD
#endif

#define KINC_SHUFFLE_TABLE(LANE_A1, LANE_A2, LANE_B1, LANE_B2)                                                                                                 \
	((((LANE_B2)&0x3) << 6) | (((LANE_B1)&0x3) << 4) | (((LANE_A2)&0x3) << 2) | (((LANE_A1)&0x3) << 0))

#if defined(KINC_SSE2)

// SSE_## related headers include earlier revisions, IE
// SSE2 contains all of SSE
#include <emmintrin.h>

typedef __m128 kinc_float32x4_t;
typedef __m128 kinc_float32x4_mask_t;

typedef __m128i kinc_int8x16_t;
typedef __m128i kinc_int8x16_mask_t;
typedef __m128i kinc_uint8x16_t;
typedef __m128i kinc_uint8x16_mask_t;
typedef __m128i kinc_int16x8_t;
typedef __m128i kinc_int16x8_mask_t;
typedef __m128i kinc_uint16x8_t;
typedef __m128i kinc_uint16x8_mask_t;
typedef __m128i kinc_int32x4_t;
typedef __m128i kinc_int32x4_mask_t;
typedef __m128i kinc_uint32x4_t;
typedef __m128i kinc_uint32x4_mask_t;

#elif defined(KINC_SSE)

#include <xmmintrin.h>

typedef __m128 kinc_float32x4_t;
typedef __m128 kinc_float32x4_mask_t;

typedef struct kinc_int8x16 {
	int8_t values[16];
} kinc_int8x16_t;

typedef struct kinc_uint8x16 {
	uint8_t values[16];
} kinc_uint8x16_t;

typedef struct kinc_int16x8 {
	int16_t values[8];
} kinc_int16x8_t;

typedef struct kinc_uint16x8 {
	uint16_t values[8];
} kinc_uint16x8_t;

typedef struct kinc_int32x4 {
	int32_t values[4];
} kinc_int32x4_t;

typedef struct kinc_uint32x4 {
	uint32_t values[4];
} kinc_uint32x4_t;

typedef kinc_int8x16_t kinc_int8x16_mask_t;
typedef kinc_uint8x16_t kinc_uint8x16_mask_t;
typedef kinc_int16x8_t kinc_int16x8_mask_t;
typedef kinc_uint16x8_t kinc_uint16x8_mask_t;
typedef kinc_int32x4_t kinc_int32x4_mask_t;
typedef kinc_uint32x4_t kinc_uint32x4_mask_t;

#elif defined(KINC_NEON)

#include <arm_neon.h>

typedef float32x4_t kinc_float32x4_t;
typedef uint32x4_t kinc_float32x4_mask_t;

typedef int8x16_t kinc_int8x16_t;
typedef uint8x16_t kinc_int8x16_mask_t;
typedef uint8x16_t kinc_uint8x16_t;
typedef uint8x16_t kinc_uint8x16_mask_t;
typedef int16x8_t kinc_int16x8_t;
typedef uint16x8_t kinc_int16x8_mask_t;
typedef uint16x8_t kinc_uint16x8_t;
typedef uint16x8_t kinc_uint16x8_mask_t;
typedef int32x4_t kinc_int32x4_t;
typedef uint32x4_t kinc_int32x4_mask_t;
typedef uint32x4_t kinc_uint32x4_t;
typedef uint32x4_t kinc_uint32x4_mask_t;

#elif defined(KINC_NOSIMD)

#include <kinc/math/core.h>

typedef struct kinc_float32x4 {
	float values[4];
} kinc_float32x4_t;

typedef kinc_float32x4_t kinc_float32x4_mask_t;

typedef struct kinc_int8x16 {
	int8_t values[16];
} kinc_int8x16_t;

typedef struct kinc_uint8x16 {
	uint8_t values[16];
} kinc_uint8x16_t;

typedef struct kinc_int16x8 {
	int16_t values[8];
} kinc_int16x8_t;

typedef struct kinc_uint16x8 {
	uint16_t values[8];
} kinc_uint16x8_t;

typedef struct kinc_int32x4 {
	int32_t values[4];
} kinc_int32x4_t;

typedef struct kinc_uint32x4 {
	uint32_t values[4];
} kinc_uint32x4_t;

typedef kinc_int8x16_t kinc_int8x16_mask_t;
typedef kinc_uint8x16_t kinc_uint8x16_mask_t;
typedef kinc_int16x8_t kinc_int16x8_mask_t;
typedef kinc_uint16x8_t kinc_uint16x8_mask_t;
typedef kinc_int32x4_t kinc_int32x4_mask_t;
typedef kinc_uint32x4_t kinc_uint32x4_mask_t;

#endif
