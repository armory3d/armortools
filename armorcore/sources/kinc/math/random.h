#pragma once

#include <kinc/global.h>

/*! \file random.h
    \brief Generates values which are kind of random.
*/

void kinc_random_init(int64_t seed);
int64_t kinc_random_get(void);
int64_t kinc_random_get_max(int64_t max);
int64_t kinc_random_get_in(int64_t min, int64_t max);

#ifdef KINC_IMPLEMENTATION_MATH
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <limits.h>
#include <stdlib.h>

// xoshiro256** 1.0

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t s[4] = {1, 2, 3, 4};

uint64_t next(void) {
	const uint64_t result = rotl(s[1] * 5, 7) * 9;

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}

void kinc_random_init(int64_t seed) {
	s[0] = (uint64_t)seed;
	s[1] = 2;
	s[2] = 3;
	s[3] = 4;
	s[1] = next();
	s[2] = next();
	s[3] = next();
}

int64_t kinc_random_get(void) {
	return (int64_t)next();
}

int64_t kinc_random_get_max(int64_t max) {
	return kinc_random_get() % (max + 1);
}

int64_t kinc_random_get_in(int64_t min, int64_t max) {
	int64_t value = kinc_random_get();
	return (value < -LLONG_MAX ? LLONG_MAX : llabs(value)) % (max + 1 - min) + min;
}

#endif
