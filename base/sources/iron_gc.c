
#include "iron_gc.h"

#ifdef NO_GC

#include <stdlib.h>
#include <stdint.h>

#define HEAP_SIZE 512 * 1024 * 1024
static uint8_t *heap = NULL;
static size_t heap_top = 0;

void *gc_alloc(size_t size) {
	#ifdef HEAP_SIZE
	size_t old_top = heap_top;
	heap_top += size;
	return &heap[old_top];
	#else
	return calloc(size, 1);
	#endif
}

void gc_array(void *ptr, int *length) {
}

void gc_leaf(void *ptr) {
}

void gc_root(void *ptr) {
}

void gc_unroot(void *ptr) {
}

void *gc_cut(void *ptr, size_t pos, size_t size) {
	return NULL;
}

void *gc_realloc(void *ptr, size_t size) {
	#ifdef HEAP_SIZE
	// NOTE: gc_realloc is not implemented when HEAP_SIZE is defined
	return gc_alloc(size);
	#else
	return realloc(ptr, size);
	#endif
}

void gc_free(void *ptr) {
	#ifdef HEAP_SIZE
	#else
	free(ptr);
	#endif
}

void gc_pause() {
}

void gc_resume() {
}

void gc_run() {
}

void gc_start(void *bos) {
	#ifdef HEAP_SIZE
	heap = (uint8_t *)calloc(HEAP_SIZE, 1);
	#endif
}

void gc_stop() {
}

#else

#include <gc.h>

void *gc_alloc(size_t size) {
	return _gc_calloc(size, sizeof(uint8_t));
}

void gc_array(void *ptr, int *length) {
	_gc_array(ptr, length);
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

void *gc_cut(void *ptr, size_t pos, size_t size) {
	return _gc_cut(ptr, pos, size);
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
