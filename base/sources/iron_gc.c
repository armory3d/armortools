
#include "iron_gc.h"

#ifdef NO_GC_USE_HEAP

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HEAP_SIZE 512 * 1024 * 1024
static uint8_t *heap     = NULL;
static size_t   heap_top = 0;

void *gc_alloc(size_t size) {
	size_t old_top = heap_top;
	heap_top += size;
	if (heap_top >= HEAP_SIZE) {
		printf("gc_alloc: out of memory\n");
	}
	return &heap[old_top];
}

void gc_leaf(void *ptr) {}
void gc_root(void *ptr) {}
void gc_unroot(void *ptr) {}

void *gc_realloc(void *ptr, size_t size) {
	void *new_ptr = gc_alloc(size);
	if (ptr != NULL) {
		memcpy(new_ptr, ptr, size);
	}
	return new_ptr;
}

void gc_free(void *ptr) {}
void gc_pause() {}
void gc_resume() {}
void gc_run() {}

void gc_start(void *bos) {
	heap = (uint8_t *)calloc(HEAP_SIZE, 1);
}

void gc_stop() {}

#elif defined(NO_GC)

#include <stdint.h>
#include <stdlib.h>

void *gc_alloc(size_t size) {
	return calloc(size, 1);
}

void gc_leaf(void *ptr) {}
void gc_root(void *ptr) {}
void gc_unroot(void *ptr) {}

void *gc_realloc(void *ptr, size_t size) {
	return realloc(ptr, size);
}

void gc_free(void *ptr) {
	free(ptr);
}

void gc_pause() {}
void gc_resume() {}
void gc_run() {}
void gc_start(void *bos) {}
void gc_stop() {}

#else

#include <gc.h>

void *gc_alloc(size_t size) {
	return _gc_calloc(size, sizeof(uint8_t));
}

void gc_leaf(void *ptr) {
	_gc_leaf(ptr);
}

void gc_root(void *ptr) {
	_gc_root(ptr);
}

void gc_unroot(void *ptr) {
	_gc_unroot(ptr);
}

void *gc_realloc(void *ptr, size_t size) {
	return ptr == NULL ? gc_alloc(size) : _gc_realloc(ptr, size);
}

void gc_free(void *ptr) {
	if (ptr != NULL) {
		_gc_free(ptr);
	}
}

void gc_pause() {
	_gc_pause();
}

void gc_resume() {
	_gc_resume();
}

void gc_run() {
	_gc_run();
}

void gc_start(void *bos) {
	_gc_start(bos);
}

void gc_stop() {
	_gc_stop();
}

#endif
