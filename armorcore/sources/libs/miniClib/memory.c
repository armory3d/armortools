#include "stdlib.h"

#ifdef KINC_WASM
__attribute__((import_module("imports"), import_name("js_fprintf"))) void js_fprintf(const char *format);

#define HEAP_SIZE 1024 * 1024 * 8
static unsigned char heap[HEAP_SIZE];
static size_t heap_top = 4;
#endif

#ifdef KINC_WASM
__attribute__((export_name("malloc"))) 
#endif
void *malloc(size_t size) {
#ifdef KINC_WASM
	// Align to 4 bytes to make js typed arrays work
	if (size % 4 != 0) {
		size += 4 - size % 4;
	}
	size_t old_top = heap_top;
	heap_top += size;
	if (heap_top >= HEAP_SIZE) {
		js_fprintf("malloc: out of memory");
	}
	return &heap[old_top];
#endif
	return NULL;
}

void *alloca(size_t size) {
	return NULL;
}

void *realloc(void *mem, size_t size) {
	return NULL;
}

void free(void *mem) {

}

void *memset(void *ptr, int value, size_t num) {
	unsigned char *data = (unsigned char *)ptr;
	for (size_t i = 0; i < num; ++i) {
		data[i] = (unsigned char)value;
	}
	return ptr;
}

void *memcpy(void *destination, const void *source, size_t num) {
	unsigned char *s = (unsigned char *)source;
	unsigned char *d = (unsigned char *)destination;
	for (size_t i = 0; i < num; ++i) {
		d[i] = s[i];
	}
	return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
	unsigned char *p1 = (unsigned char *)ptr1;
	unsigned char *p2 = (unsigned char *)ptr2;
	for (size_t i = 0; i < num; ++i) {
		if (p1[i] != p2[i]) {
			return (int)p1[i] - (int)p2[i];
		}
	}
	return 0;
}
