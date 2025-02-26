#pragma once

// gc - A simple mark and sweep garbage collector for C.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void _gc_start(void *bos);
size_t _gc_stop();
void _gc_pause();
void _gc_resume();
size_t _gc_run();
void *_gc_calloc(size_t count, size_t size);
void _gc_array(void *ptr, int *length);
void _gc_leaf(void *ptr);
void _gc_root(void *ptr);
void _gc_unroot(void *ptr);
void *_gc_cut(void *ptr, size_t pos, size_t size);
void *_gc_realloc(void *ptr, size_t size);
void _gc_free(void *ptr);
