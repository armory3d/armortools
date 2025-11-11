#include "stdlib.h"

int system(const char *string) {
	return 0;
}

void exit(int code) {
	exit(code);
}

long int strtol(const char *str, char **endptr, int base) {
	return 0;
}

float strtof(const char *str, char **endptr) {
	return 0.0f;
}

double atof (const char* str) {
	return 0.0;
}

int abs(int n) {
	return n < 0 ? -n : n;
}

long long int llabs(long long int n) {
	return n < 0 ? -n : n;
}

void qsort(void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {}

int rand() {
	return 0;
}
