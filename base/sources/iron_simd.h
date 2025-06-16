#pragma once

#include <iron_global.h>
#include <string.h>

/*! \file float32x4.h
    \brief Provides 128bit four-element floating point SIMD operations which are mapped to equivalent SSE or Neon operations.
*/

// Any level of AVX Capability (Could be AVX, AVX2, AVX512, etc.)
//(Currently) only used for checking existence of earlier SSE instruction sets
#if defined(__AVX__)
// Unfortunate situation here
// MSVC does not provide compiletime macros for the following instruction sets
// but their existence is implied by AVX and higher
#define IRON_SSE4_2
#define IRON_SSE4_1
#define IRON_SSSE3
#define IRON_SSE3
#endif

// SSE2 Capability check
// Note for Windows:
//	_M_IX86_FP checks SSE2 and SSE for 32bit Windows programs only, and is unset if not a 32bit program.
//	SSE2 and earlier is --guaranteed-- to be active for any 64bit Windows program
#if defined(__SSE2__) || (_M_IX86_FP == 2) || (defined(IRON_WINDOWS) && defined(IRON_64))
#define IRON_SSE2
#endif

// SSE Capability check
#if defined(__SSE__) || _M_IX86_FP == 2 || _M_IX86_FP == 1 || (defined(IRON_WINDOWS) && !defined(__aarch64__)) ||                                              \
    (defined(IRON_WINDOWSAPP) && !defined(__aarch64__)) || (defined(IRON_MACOS) && __x86_64)

#define IRON_SSE
#endif

// NEON Capability check
#if (defined(IRON_IOS) || defined(__aarch64__)) && !defined(IRON_NOSIMD)
#define IRON_NEON
#endif

// No SIMD Capabilities
#if !defined(IRON_SSE4_2) && !defined(IRON_SSE4_1) && !defined(IRON_SSSE3) && !defined(IRON_SSE3) && !defined(IRON_SSE2) && !defined(IRON_SSE) &&              \
    !defined(IRON_NEON) && !defined(IRON_NOSIMD)

#define IRON_NOSIMD
#endif

#define IRON_SHUFFLE_TABLE(LANE_A1, LANE_A2, LANE_B1, LANE_B2)                                                                                                 \
	((((LANE_B2)&0x3) << 6) | (((LANE_B1)&0x3) << 4) | (((LANE_A2)&0x3) << 2) | (((LANE_A1)&0x3) << 0))

#if defined(IRON_SSE2)

// SSE_## related headers include earlier revisions, IE
// SSE2 contains all of SSE
#include <emmintrin.h>

typedef __m128 iron_float32x4_t;
typedef __m128 iron_float32x4_mask_t;

#elif defined(IRON_SSE)

#include <xmmintrin.h>

typedef __m128 iron_float32x4_t;
typedef __m128 iron_float32x4_mask_t;

#elif defined(IRON_NEON)

#include <arm_neon.h>

typedef float32x4_t iron_float32x4_t;
typedef uint32x4_t iron_float32x4_mask_t;

#elif defined(IRON_NOSIMD)

#include <iron_math.h>

typedef struct iron_float32x4 {
	float values[4];
} iron_float32x4_t;

typedef iron_float32x4_t iron_float32x4_mask_t;

#endif

#if defined(IRON_SSE)

static inline iron_float32x4_t iron_float32x4_intrin_load(const float *values) {
	return _mm_load_ps(values);
}

static inline iron_float32x4_t iron_float32x4_intrin_load_unaligned(const float *values) {
	return _mm_loadu_ps(values);
}

static inline iron_float32x4_t iron_float32x4_load(float a, float b, float c, float d) {
	return _mm_set_ps(d, c, b, a);
}

static inline iron_float32x4_t iron_float32x4_load_all(float t) {
	return _mm_set_ps1(t);
}

static inline void iron_float32x4_store(float *destination, iron_float32x4_t value) {
	_mm_store_ps(destination, value);
}

static inline void iron_float32x4_store_unaligned(float *destination, iron_float32x4_t value) {
	_mm_storeu_ps(destination, value);
}

static inline float iron_float32x4_get(iron_float32x4_t t, int index) {
	union {
		__m128 value;
		float elements[4];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

static inline iron_float32x4_t iron_float32x4_abs(iron_float32x4_t t) {
	__m128 mask = _mm_set_ps1(-0.f);
	return _mm_andnot_ps(mask, t);
}

static inline iron_float32x4_t iron_float32x4_add(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_add_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_div(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_div_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_mul(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_mul_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_neg(iron_float32x4_t t) {
	__m128 negative = _mm_set_ps1(-1.0f);
	return _mm_mul_ps(t, negative);
}

static inline iron_float32x4_t iron_float32x4_reciprocal_approximation(iron_float32x4_t t) {
	return _mm_rcp_ps(t);
}

static inline iron_float32x4_t iron_float32x4_reciprocal_sqrt_approximation(iron_float32x4_t t) {
	return _mm_rsqrt_ps(t);
}

static inline iron_float32x4_t iron_float32x4_sub(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_sub_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_sqrt(iron_float32x4_t t) {
	return _mm_sqrt_ps(t);
}

static inline iron_float32x4_t iron_float32x4_max(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_max_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_min(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_min_ps(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpeq(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_cmpeq_ps(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpge(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_cmpge_ps(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpgt(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_cmpgt_ps(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmple(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_cmple_ps(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmplt(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_cmplt_ps(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpneq(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_cmpneq_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_sel(iron_float32x4_t a, iron_float32x4_t b, iron_float32x4_mask_t mask) {
	return _mm_xor_ps(b, _mm_and_ps(mask, _mm_xor_ps(a, b)));
}

static inline iron_float32x4_t iron_float32x4_or(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_or_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_and(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_and_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_xor(iron_float32x4_t a, iron_float32x4_t b) {
	return _mm_xor_ps(a, b);
}

static inline iron_float32x4_t iron_float32x4_not(iron_float32x4_t t) {
	__m128 zeroes = _mm_setzero_ps();
	return _mm_xor_ps(t, _mm_cmpeq_ps(zeroes, zeroes));
}

#define iron_float32x4_shuffle_custom(abcd, efgh, left_1, left_2, right_1, right_2)                                                                            \
	_mm_shuffle_ps((abcd), (efgh), IRON_SHUFFLE_TABLE((left_1), (left_2), (right_1), (right_2)))

static inline iron_float32x4_t iron_float32x4_shuffle_aebf(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	// aka unpacklo aka zip1 aka interleave low
	return _mm_unpacklo_ps(abcd, efgh);
}

static inline iron_float32x4_t iron_float32x4_shuffle_cgdh(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	// aka unpackhi aka zip2 aka interleave high
	return _mm_unpackhi_ps(abcd, efgh);
}

static inline iron_float32x4_t iron_float32x4_shuffle_abef(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	// aka movelh
	return _mm_movelh_ps(abcd, efgh);
}

static inline iron_float32x4_t iron_float32x4_shuffle_ghcd(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	// aka movehl
	return _mm_movehl_ps(abcd, efgh);
}

#elif defined(IRON_NEON)

static inline iron_float32x4_t iron_float32x4_intrin_load(const float *values) {
	return vld1q_f32(values);
}

static inline iron_float32x4_t iron_float32x4_intrin_load_unaligned(const float *values) {
	return iron_float32x4_intrin_load(values);
}

static inline iron_float32x4_t iron_float32x4_load(float a, float b, float c, float d) {
	return (iron_float32x4_t){a, b, c, d};
}

static inline iron_float32x4_t iron_float32x4_load_all(float t) {
	return (iron_float32x4_t){t, t, t, t};
}

static inline void iron_float32x4_store(float *destination, iron_float32x4_t value) {
	vst1q_f32(destination, value);
}

static inline void iron_float32x4_store_unaligned(float *destination, iron_float32x4_t value) {
	iron_float32x4_store(destination, value);
}

static inline float iron_float32x4_get(iron_float32x4_t t, int index) {
	return t[index];
}

static inline iron_float32x4_t iron_float32x4_abs(iron_float32x4_t t) {
	return vabsq_f32(t);
}

static inline iron_float32x4_t iron_float32x4_add(iron_float32x4_t a, iron_float32x4_t b) {
	return vaddq_f32(a, b);
}

static inline iron_float32x4_t iron_float32x4_div(iron_float32x4_t a, iron_float32x4_t b) {
#if defined(__aarch64__)
	return vdivq_f32(a, b);
#else
	float32x4_t inv = vrecpeq_f32(b);
	float32x4_t restep = vrecpsq_f32(b, inv);
	inv = vmulq_f32(restep, inv);
	return vmulq_f32(a, inv);
#endif
}

static inline iron_float32x4_t iron_float32x4_mul(iron_float32x4_t a, iron_float32x4_t b) {
	return vmulq_f32(a, b);
}

static inline iron_float32x4_t iron_float32x4_neg(iron_float32x4_t t) {
	return vnegq_f32(t);
}

static inline iron_float32x4_t iron_float32x4_reciprocal_approximation(iron_float32x4_t t) {
	return vrecpeq_f32(t);
}

static inline iron_float32x4_t iron_float32x4_reciprocal_sqrt_approximation(iron_float32x4_t t) {
	return vrsqrteq_f32(t);
}

static inline iron_float32x4_t iron_float32x4_sub(iron_float32x4_t a, iron_float32x4_t b) {
	return vsubq_f32(a, b);
}

static inline iron_float32x4_t iron_float32x4_sqrt(iron_float32x4_t t) {
#if defined(__aarch64__)
	return vsqrtq_f32(t);
#else
	return vmulq_f32(t, vrsqrteq_f32(t));
#endif
}

static inline iron_float32x4_t iron_float32x4_max(iron_float32x4_t a, iron_float32x4_t b) {
	return vmaxq_f32(a, b);
}

static inline iron_float32x4_t iron_float32x4_min(iron_float32x4_t a, iron_float32x4_t b) {
	return vminq_f32(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpeq(iron_float32x4_t a, iron_float32x4_t b) {
	return vceqq_f32(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpge(iron_float32x4_t a, iron_float32x4_t b) {
	return vcgeq_f32(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpgt(iron_float32x4_t a, iron_float32x4_t b) {
	return vcgtq_f32(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmple(iron_float32x4_t a, iron_float32x4_t b) {
	return vcleq_f32(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmplt(iron_float32x4_t a, iron_float32x4_t b) {
	return vcltq_f32(a, b);
}

static inline iron_float32x4_mask_t iron_float32x4_cmpneq(iron_float32x4_t a, iron_float32x4_t b) {
	return vmvnq_u32(vceqq_f32(a, b));
}

static inline iron_float32x4_t iron_float32x4_sel(iron_float32x4_t a, iron_float32x4_t b, iron_float32x4_mask_t mask) {
	return vbslq_f32(mask, a, b);
}

static inline iron_float32x4_t iron_float32x4_or(iron_float32x4_t a, iron_float32x4_t b) {
	uint32x4_t acvt = vreinterpretq_u32_f32(a);
	uint32x4_t bcvt = vreinterpretq_u32_f32(b);

	return vreinterpretq_f32_u32(vorrq_u32(acvt, bcvt));
}

static inline iron_float32x4_t iron_float32x4_and(iron_float32x4_t a, iron_float32x4_t b) {
	uint32x4_t acvt = vreinterpretq_u32_f32(a);
	uint32x4_t bcvt = vreinterpretq_u32_f32(b);

	return vreinterpretq_f32_u32(vandq_u32(acvt, bcvt));
}

static inline iron_float32x4_t iron_float32x4_xor(iron_float32x4_t a, iron_float32x4_t b) {
	uint32x4_t acvt = vreinterpretq_u32_f32(a);
	uint32x4_t bcvt = vreinterpretq_u32_f32(b);

	return vreinterpretq_f32_u32(veorq_u32(acvt, bcvt));
}

static inline iron_float32x4_t iron_float32x4_not(iron_float32x4_t t) {
	uint32x4_t tcvt = vreinterpretq_u32_f32(t);

	return vreinterpretq_f32_u32(vmvnq_u32(tcvt));
}

#define iron_float32x4_shuffle_custom(abcd, efgh, left_1, left_2, right_1, right_2)                                                                            \
	(iron_float32x4_t) {                                                                                                                                       \
		vgetq_lane_f32((abcd), ((left_1)&0x3)), vgetq_lane_f32((abcd), ((left_2)&0x3)), vgetq_lane_f32((efgh), ((right_1)&0x3)),                               \
		    vgetq_lane_f32((efgh), ((right_2)&0x3))                                                                                                            \
	}

static inline iron_float32x4_t iron_float32x4_shuffle_aebf(iron_float32x4_t abcd, iron_float32x4_t efgh) {
#if defined(__aarch64__)

	return vzip1q_f32(abcd, efgh);

#else

	float a = vgetq_lane_f32(abcd, 0);
	float b = vgetq_lane_f32(abcd, 1);
	float e = vgetq_lane_f32(efgh, 0);
	float f = vgetq_lane_f32(efgh, 1);

	return (iron_float32x4_t){a, e, b, f};

#endif
}

static inline iron_float32x4_t iron_float32x4_shuffle_cgdh(iron_float32x4_t abcd, iron_float32x4_t efgh) {
#if defined(__aarch64__)

	return vzip2q_f32(abcd, efgh);

#else

	float c = vgetq_lane_f32(abcd, 2);
	float d = vgetq_lane_f32(abcd, 3);
	float g = vgetq_lane_f32(efgh, 2);
	float h = vgetq_lane_f32(efgh, 3);

	return (iron_float32x4_t){c, g, d, h};

#endif
}

static inline iron_float32x4_t iron_float32x4_shuffle_abef(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	float32x2_t ab = vget_low_f32(abcd);
	float32x2_t ef = vget_low_f32(efgh);

	return vcombine_f32(ab, ef);
}

static inline iron_float32x4_t iron_float32x4_shuffle_ghcd(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	float32x2_t cd = vget_high_f32(abcd);
	float32x2_t gh = vget_high_f32(efgh);

	return vcombine_f32(gh, cd);
}

#else

#include <math.h>

static inline iron_float32x4_t iron_float32x4_intrin_load(const float *values) {
	iron_float32x4_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_intrin_load_unaligned(const float *values) {
	return iron_float32x4_intrin_load(values);
}

static inline iron_float32x4_t iron_float32x4_load(float a, float b, float c, float d) {
	iron_float32x4_t value;
	value.values[0] = a;
	value.values[1] = b;
	value.values[2] = c;
	value.values[3] = d;
	return value;
}

static inline iron_float32x4_t iron_float32x4_load_all(float t) {
	iron_float32x4_t value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	return value;
}

static inline void iron_float32x4_store(float *destination, iron_float32x4_t value) {
	destination[0] = value.values[0];
	destination[1] = value.values[1];
	destination[2] = value.values[2];
	destination[3] = value.values[3];
}

static inline void iron_float32x4_store_unaligned(float *destination, iron_float32x4_t value) {
	iron_float32x4_store(destination, value);
}

static inline float iron_float32x4_get(iron_float32x4_t t, int index) {
	return t.values[index];
}

static inline iron_float32x4_t iron_float32x4_abs(iron_float32x4_t t) {
	iron_float32x4_t value;
	value.values[0] = iron_abs(t.values[0]);
	value.values[1] = iron_abs(t.values[1]);
	value.values[2] = iron_abs(t.values[2]);
	value.values[3] = iron_abs(t.values[3]);
	return value;
}

static inline iron_float32x4_t iron_float32x4_add(iron_float32x4_t a, iron_float32x4_t b) {
	iron_float32x4_t value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_div(iron_float32x4_t a, iron_float32x4_t b) {
	iron_float32x4_t value;
	value.values[0] = a.values[0] / b.values[0];
	value.values[1] = a.values[1] / b.values[1];
	value.values[2] = a.values[2] / b.values[2];
	value.values[3] = a.values[3] / b.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_mul(iron_float32x4_t a, iron_float32x4_t b) {
	iron_float32x4_t value;
	value.values[0] = a.values[0] * b.values[0];
	value.values[1] = a.values[1] * b.values[1];
	value.values[2] = a.values[2] * b.values[2];
	value.values[3] = a.values[3] * b.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_neg(iron_float32x4_t t) {
	iron_float32x4_t value;
	value.values[0] = -t.values[0];
	value.values[1] = -t.values[1];
	value.values[2] = -t.values[2];
	value.values[3] = -t.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_reciprocal_approximation(iron_float32x4_t t) {
	iron_float32x4_t value;
	value.values[0] = 1.0f / t.values[0];
	value.values[1] = 1.0f / t.values[1];
	value.values[2] = 1.0f / t.values[2];
	value.values[3] = 1.0f / t.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_reciprocal_sqrt_approximation(iron_float32x4_t t) {
	iron_float32x4_t value;
	value.values[0] = 1.0f / sqrtf(t.values[0]);
	value.values[1] = 1.0f / sqrtf(t.values[1]);
	value.values[2] = 1.0f / sqrtf(t.values[2]);
	value.values[3] = 1.0f / sqrtf(t.values[3]);
	return value;
}

static inline iron_float32x4_t iron_float32x4_sub(iron_float32x4_t a, iron_float32x4_t b) {
	iron_float32x4_t value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_sqrt(iron_float32x4_t t) {
	iron_float32x4_t value;
	value.values[0] = sqrtf(t.values[0]);
	value.values[1] = sqrtf(t.values[1]);
	value.values[2] = sqrtf(t.values[2]);
	value.values[3] = sqrtf(t.values[3]);
	return value;
}

static inline iron_float32x4_t iron_float32x4_max(iron_float32x4_t a, iron_float32x4_t b) {
	iron_float32x4_t value;
	value.values[0] = iron_max(a.values[0], b.values[0]);
	value.values[1] = iron_max(a.values[1], b.values[1]);
	value.values[2] = iron_max(a.values[2], b.values[2]);
	value.values[3] = iron_max(a.values[3], b.values[3]);
	return value;
}

static inline iron_float32x4_t iron_float32x4_min(iron_float32x4_t a, iron_float32x4_t b) {
	iron_float32x4_t value;
	value.values[0] = iron_min(a.values[0], b.values[0]);
	value.values[1] = iron_min(a.values[1], b.values[1]);
	value.values[2] = iron_min(a.values[2], b.values[2]);
	value.values[3] = iron_min(a.values[3], b.values[3]);
	return value;
}

static inline iron_float32x4_mask_t iron_float32x4_cmpeq(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t mask_cvt[4];
	mask_cvt[0] = a.values[0] == b.values[0] ? 0xffffffff : 0;
	mask_cvt[1] = a.values[1] == b.values[1] ? 0xffffffff : 0;
	mask_cvt[2] = a.values[2] == b.values[2] ? 0xffffffff : 0;
	mask_cvt[3] = a.values[3] == b.values[3] ? 0xffffffff : 0;

	iron_float32x4_mask_t mask;
	memcpy(&mask.values[0], &mask_cvt[0], sizeof(mask_cvt));

	return mask;
}

static inline iron_float32x4_mask_t iron_float32x4_cmpge(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t mask_cvt[4];
	mask_cvt[0] = a.values[0] >= b.values[0] ? 0xffffffff : 0;
	mask_cvt[1] = a.values[1] >= b.values[1] ? 0xffffffff : 0;
	mask_cvt[2] = a.values[2] >= b.values[2] ? 0xffffffff : 0;
	mask_cvt[3] = a.values[3] >= b.values[3] ? 0xffffffff : 0;

	iron_float32x4_mask_t mask;
	memcpy(&mask.values[0], &mask_cvt[0], sizeof(mask_cvt));

	return mask;
}

static inline iron_float32x4_mask_t iron_float32x4_cmpgt(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t mask_cvt[4];
	mask_cvt[0] = a.values[0] > b.values[0] ? 0xffffffff : 0;
	mask_cvt[1] = a.values[1] > b.values[1] ? 0xffffffff : 0;
	mask_cvt[2] = a.values[2] > b.values[2] ? 0xffffffff : 0;
	mask_cvt[3] = a.values[3] > b.values[3] ? 0xffffffff : 0;

	iron_float32x4_mask_t mask;
	memcpy(&mask.values[0], &mask_cvt[0], sizeof(mask_cvt));

	return mask;
}

static inline iron_float32x4_mask_t iron_float32x4_cmple(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t mask_cvt[4];
	mask_cvt[0] = a.values[0] <= b.values[0] ? 0xffffffff : 0;
	mask_cvt[1] = a.values[1] <= b.values[1] ? 0xffffffff : 0;
	mask_cvt[2] = a.values[2] <= b.values[2] ? 0xffffffff : 0;
	mask_cvt[3] = a.values[3] <= b.values[3] ? 0xffffffff : 0;

	iron_float32x4_mask_t mask;
	memcpy(&mask.values[0], &mask_cvt[0], sizeof(mask_cvt));

	return mask;
}

static inline iron_float32x4_mask_t iron_float32x4_cmplt(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t mask_cvt[4];
	mask_cvt[0] = a.values[0] < b.values[0] ? 0xffffffff : 0;
	mask_cvt[1] = a.values[1] < b.values[1] ? 0xffffffff : 0;
	mask_cvt[2] = a.values[2] < b.values[2] ? 0xffffffff : 0;
	mask_cvt[3] = a.values[3] < b.values[3] ? 0xffffffff : 0;

	iron_float32x4_mask_t mask;
	memcpy(&mask.values[0], &mask_cvt[0], sizeof(mask_cvt));

	return mask;
}

static inline iron_float32x4_mask_t iron_float32x4_cmpneq(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t mask_cvt[4];
	mask_cvt[0] = a.values[0] != b.values[0] ? 0xffffffff : 0;
	mask_cvt[1] = a.values[1] != b.values[1] ? 0xffffffff : 0;
	mask_cvt[2] = a.values[2] != b.values[2] ? 0xffffffff : 0;
	mask_cvt[3] = a.values[3] != b.values[3] ? 0xffffffff : 0;

	iron_float32x4_mask_t mask;
	memcpy(&mask.values[0], &mask_cvt[0], sizeof(mask_cvt));

	return mask;
}

static inline iron_float32x4_t iron_float32x4_sel(iron_float32x4_t a, iron_float32x4_t b, iron_float32x4_mask_t mask) {
	iron_float32x4_t value;
	value.values[0] = mask.values[0] != 0.0f ? a.values[0] : b.values[0];
	value.values[1] = mask.values[1] != 0.0f ? a.values[1] : b.values[1];
	value.values[2] = mask.values[2] != 0.0f ? a.values[2] : b.values[2];
	value.values[3] = mask.values[3] != 0.0f ? a.values[3] : b.values[3];
	return value;
}

static inline iron_float32x4_t iron_float32x4_or(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t acvt[4];
	uint32_t bcvt[4];
	memcpy(&acvt[0], &a.values[0], sizeof(a));
	memcpy(&bcvt[0], &b.values[0], sizeof(b));

	acvt[0] |= bcvt[0];
	acvt[1] |= bcvt[1];
	acvt[2] |= bcvt[2];
	acvt[3] |= bcvt[3];

	iron_float32x4_t value;
	memcpy(&value.values[0], &acvt[0], sizeof(acvt));

	return value;
}

static inline iron_float32x4_t iron_float32x4_and(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t acvt[4];
	uint32_t bcvt[4];
	memcpy(&acvt[0], &a.values[0], sizeof(a));
	memcpy(&bcvt[0], &b.values[0], sizeof(b));

	acvt[0] &= bcvt[0];
	acvt[1] &= bcvt[1];
	acvt[2] &= bcvt[2];
	acvt[3] &= bcvt[3];

	iron_float32x4_t value;
	memcpy(&value.values[0], &acvt[0], sizeof(acvt));

	return value;
}

static inline iron_float32x4_t iron_float32x4_xor(iron_float32x4_t a, iron_float32x4_t b) {
	uint32_t acvt[4];
	uint32_t bcvt[4];
	memcpy(&acvt[0], &a.values[0], sizeof(a));
	memcpy(&bcvt[0], &b.values[0], sizeof(b));

	acvt[0] ^= bcvt[0];
	acvt[1] ^= bcvt[1];
	acvt[2] ^= bcvt[2];
	acvt[3] ^= bcvt[3];

	iron_float32x4_t value;
	memcpy(&value.values[0], &acvt[0], sizeof(acvt));

	return value;
}

static inline iron_float32x4_t iron_float32x4_not(iron_float32x4_t t) {
	uint32_t tcvt[4];
	memcpy(&tcvt[0], &t.values[0], sizeof(t));

	tcvt[0] = ~tcvt[0];
	tcvt[1] = ~tcvt[1];
	tcvt[2] = ~tcvt[2];
	tcvt[3] = ~tcvt[3];

	iron_float32x4_t value;
	memcpy(&value.values[0], &tcvt[0], sizeof(tcvt));

	return value;
}

static inline iron_float32x4_t iron_float32x4_shuffle_custom(iron_float32x4_t abcd, iron_float32x4_t efgh, const uint32_t left_1, const uint32_t left_2,
                                                             const uint32_t right_1, const uint32_t right_2) {
	iron_float32x4_t value;

	value.values[0] = abcd.values[left_1 & 0x3];
	value.values[1] = abcd.values[left_2 & 0x3];
	value.values[2] = efgh.values[right_1 & 0x3];
	value.values[3] = efgh.values[right_2 & 0x3];

	return value;
}

static inline iron_float32x4_t iron_float32x4_shuffle_aebf(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	iron_float32x4_t value;

	value.values[0] = abcd.values[0];
	value.values[1] = efgh.values[0];
	value.values[2] = abcd.values[1];
	value.values[3] = efgh.values[1];

	return value;
}

static inline iron_float32x4_t iron_float32x4_shuffle_cgdh(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	iron_float32x4_t value;

	value.values[0] = abcd.values[2];
	value.values[1] = efgh.values[2];
	value.values[2] = abcd.values[3];
	value.values[3] = efgh.values[3];

	return value;
}

static inline iron_float32x4_t iron_float32x4_shuffle_abef(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	iron_float32x4_t value;

	value.values[0] = abcd.values[0];
	value.values[1] = abcd.values[1];
	value.values[2] = efgh.values[0];
	value.values[3] = efgh.values[1];

	return value;
}

static inline iron_float32x4_t iron_float32x4_shuffle_ghcd(iron_float32x4_t abcd, iron_float32x4_t efgh) {
	iron_float32x4_t value;

	value.values[0] = efgh.values[2];
	value.values[1] = efgh.values[3];
	value.values[2] = abcd.values[2];
	value.values[3] = abcd.values[3];

	return value;
}

#endif
