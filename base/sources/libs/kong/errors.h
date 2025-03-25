#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// #if (__STDC_VERSION__ >= 201112L)
// #define noreturn _Noreturn
// #else
#define noreturn
// #endif

typedef struct debug_context {
	const char *filename;
	uint32_t    column;
	uint32_t    line;
} debug_context;

noreturn void error(debug_context context, const char *message, ...);
noreturn void error_no_context(const char *message, ...);
noreturn void error_args(debug_context context, const char *message, va_list args);
noreturn void error_args_no_context(const char *message, va_list args);
void          check_function(bool test, debug_context context, const char *message, ...);
#define check(test, context, message, ...) \
	assert(test);                          \
	check_function(test, context, message, ##__VA_ARGS__)
void check_args(bool test, debug_context context, const char *message, va_list args);

// V_ASSERT_CONTRACT, assertMacro:check

#ifdef __cplusplus
}
#endif
