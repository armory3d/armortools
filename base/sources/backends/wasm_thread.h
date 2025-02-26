#pragma once

typedef struct {
	int nothing;
} kinc_thread_impl_t;

typedef struct {
	int nothing;
} kinc_mutex_impl_t;

#pragma once

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue)

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue)

#define KINC_ATOMIC_INCREMENT(pointer)

#define KINC_ATOMIC_DECREMENT(pointer)

#define KINC_ATOMIC_EXCHANGE_32(pointer, value)

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value)
