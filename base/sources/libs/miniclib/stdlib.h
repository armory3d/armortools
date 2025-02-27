#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long size_t;
#define EXIT_FAILURE 1

void *malloc(size_t size);

void *alloca(size_t size);

void *realloc(void *mem, size_t size);

void free(void *mem);

void exit(int code);

long int strtol(const char *str, char **endptr, int base);

int abs(int n);

long long int llabs(long long int n);

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *));

#ifdef __cplusplus
}
#endif
