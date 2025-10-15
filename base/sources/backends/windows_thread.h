#pragma once

#include <intrin.h>

typedef struct {
	void *handle;
	void *param;
	void (*func)(void *param);
} iron_thread_impl_t;

typedef struct {
	void *DebugInfo;
	long  LockCount;
	long  RecursionCount;
	void *OwningThread;
	void *LockSemaphore;
	unsigned long __w64 SpinCount;
} iron_microsoft_critical_section_t;

typedef struct {
	iron_microsoft_critical_section_t criticalSection;
} iron_mutex_impl_t;

static inline bool iron_atomic_compare_exchange(volatile int32_t *pointer, int32_t old_value, int32_t new_value) {
	return _InterlockedCompareExchange((volatile long *)pointer, new_value, old_value) == old_value;
}

#define IRON_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) (iron_atomic_compare_exchange(pointer, oldValue, newValue))

static inline bool iron_atomic_compare_exchange_pointer(void *volatile *pointer, void *old_value, void *new_value) {
	return _InterlockedCompareExchangePointer(pointer, new_value, old_value) == old_value;
}

#define IRON_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) (iron_atomic_compare_exchange_pointer(pointer, oldValue, newValue))

static inline int32_t iron_atomic_increment(volatile int32_t *pointer) {
	return _InterlockedIncrement((volatile long *)pointer) - 1;
}

#define IRON_ATOMIC_INCREMENT(pointer) (iron_atomic_increment(pointer))

static inline int32_t iron_atomic_decrement(volatile int32_t *pointer) {
	return _InterlockedDecrement((volatile long *)pointer) + 1;
}

#define IRON_ATOMIC_DECREMENT(pointer) (iron_atomic_decrement(pointer))

static inline void iron_atomic_exchange(volatile int32_t *pointer, int32_t value) {
	_InterlockedExchange((volatile long *)pointer, value);
}

#define IRON_ATOMIC_EXCHANGE_32(pointer, value) (iron_atomic_exchange(pointer, value))

static inline void iron_atomic_exchange_float(volatile float *pointer, float value) {
	_InterlockedExchange((volatile long *)pointer, *(long *)&value);
}

#define IRON_ATOMIC_EXCHANGE_FLOAT(pointer, value) (iron_atomic_exchange_float(pointer, value))

static inline void iron_atomic_exchange_double(volatile double *pointer, double value) {
	_InterlockedExchange64((volatile __int64 *)pointer, *(__int64 *)&value);
}

#define IRON_ATOMIC_EXCHANGE_DOUBLE(pointer, value) (iron_atomic_exchange_double(pointer, value))
