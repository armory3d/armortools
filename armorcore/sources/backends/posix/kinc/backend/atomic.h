#pragma once

#include <kinc/global.h>

#if defined(KINC_MACOS) || defined(KINC_IOS)

#include <libkern/OSAtomic.h>

static inline bool kinc_atomic_compare_exchange(volatile int32_t *pointer, int32_t old_value, int32_t new_value) {
	return OSAtomicCompareAndSwap32Barrier(old_value, new_value, pointer);
}

static inline bool kinc_atomic_compare_exchange_pointer(void *volatile *pointer, void *old_value, void *new_value) {
	return OSAtomicCompareAndSwapPtrBarrier(old_value, new_value, pointer);
}

static inline int32_t kinc_atomic_increment(volatile int32_t *pointer) {
	return OSAtomicIncrement32Barrier(pointer) - 1;
}

static inline int32_t kinc_atomic_decrement(volatile int32_t *pointer) {
	return OSAtomicDecrement32Barrier(pointer) + 1;
}

static inline void kinc_atomic_exchange(volatile int32_t *pointer, int32_t value) {
	__sync_swap(pointer, value);
}

static inline void kinc_atomic_exchange_float(volatile float *pointer, float value) {
	__sync_swap((volatile int32_t *)pointer, *(int32_t *)&value);
}

static inline void kinc_atomic_exchange_double(volatile double *pointer, double value) {
	__sync_swap((volatile int64_t *)pointer, *(int64_t *)&value);
}

#else

// clang/gcc intrinsics

static inline bool kinc_atomic_compare_exchange(volatile int32_t *pointer, int32_t old_value, int32_t new_value) {
	return __sync_val_compare_and_swap(pointer, old_value, new_value) == old_value;
}

static inline bool kinc_atomic_compare_exchange_pointer(void *volatile *pointer, void *old_value, void *new_value) {
	return __sync_val_compare_and_swap(pointer, old_value, new_value) == old_value;
}

static inline int32_t kinc_atomic_increment(volatile int32_t *pointer) {
	return __sync_fetch_and_add(pointer, 1);
}

static inline int32_t kinc_atomic_decrement(volatile int32_t *pointer) {
	return __sync_fetch_and_sub(pointer, 1);
}

#ifdef __clang__

static inline void kinc_atomic_exchange(volatile int32_t *pointer, int32_t value) {
	__sync_swap(pointer, value);
}

static inline void kinc_atomic_exchange_float(volatile float *pointer, float value) {
	__sync_swap((volatile int32_t *)pointer, *(int32_t *)&value);
}

static inline void kinc_atomic_exchange_double(volatile double *pointer, double value) {
	__sync_swap((volatile int64_t *)pointer, *(int64_t *)&value);
}

#else

// Beware, __sync_lock_test_and_set is not a full barrier and can have platform-specific weirdness

static inline void kinc_atomic_exchange(volatile int32_t *pointer, int32_t value) {
	__sync_lock_test_and_set(pointer, value);
}

static inline void kinc_atomic_exchange_float(volatile float *pointer, float value) {
	__sync_lock_test_and_set((volatile int32_t *)pointer, *(int32_t *)&value);
}

static inline void kinc_atomic_exchange_double(volatile double *pointer, double value) {
	__sync_lock_test_and_set((volatile int64_t *)pointer, *(int64_t *)&value);
}

#endif

#endif

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) (kinc_atomic_compare_exchange(pointer, oldValue, newValue))

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) (kinc_atomic_compare_exchange_pointer(pointer, oldValue, newValue))

#define KINC_ATOMIC_INCREMENT(pointer) (kinc_atomic_increment(pointer))

#define KINC_ATOMIC_DECREMENT(pointer) (kinc_atomic_decrement(pointer))

#define KINC_ATOMIC_EXCHANGE_32(pointer, value) (kinc_atomic_exchange(pointer, value))

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value) (kinc_atomic_exchange_float(pointer, value))

#define KINC_ATOMIC_EXCHANGE_DOUBLE(pointer, value) (kinc_atomic_exchange_double(pointer, value))
