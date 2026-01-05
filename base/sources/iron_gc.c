
#include "iron_gc.h"

#ifdef NO_GC

#include <stdint.h>
#include <stdlib.h>

void *gc_alloc(size_t size) {
	return calloc(size, 1);
}

void gc_array(void *ptr, uint32_t *length) {}
void gc_leaf(void *ptr) {}
void gc_root(void *ptr) {}
void gc_unroot(void *ptr) {}

void *gc_cut(void *ptr, size_t pos, size_t size) {
	return NULL;
}

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

void gc_array(void *ptr, uint32_t *length) {
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
