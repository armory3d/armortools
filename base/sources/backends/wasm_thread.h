#pragma once

typedef struct {
	int nothing;
} iron_thread_impl_t;

typedef struct {
	int nothing;
} iron_mutex_impl_t;

#pragma once

#define IRON_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue)

#define IRON_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue)

#define IRON_ATOMIC_INCREMENT(pointer)

#define IRON_ATOMIC_DECREMENT(pointer)

#define IRON_ATOMIC_EXCHANGE_32(pointer, value)

#define IRON_ATOMIC_EXCHANGE_FLOAT(pointer, value)
