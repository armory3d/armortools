// MIT License

// Copyright (c) 2019 Marc Kirchner

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// https://github.com/mkirchner/gc

#include "gc.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
// #include <kinc/log.h>
// #include <kinc/system.h>

#define PTRSIZE sizeof(char *)

#define GC_TAG_NONE 0x0
#define GC_TAG_LEAF 0x1
#define GC_TAG_MARK 0x2
#define GC_TAG_ROOT 0x4
#define GC_TAG_CUT 0x8

#if defined(_MSC_VER) && !defined(__clang__)
#define __builtin_frame_address(x)  ((void)(x), _AddressOfReturnAddress())
#endif

typedef struct gc_allocation {
	void *ptr; // mem pointer
	size_t size; // allocated size in bytes
	int *array_length; // if this alloc is an array
	char tag; // the tag for mark-and-sweep
	char roots; // number of root users
	struct gc_allocation *next; // separate chaining
	struct gc_allocation *cut;
} gc_allocation_t;

typedef struct gc_allocation_map {
	size_t capacity;
	size_t min_capacity;
	double downsize_factor;
	double upsize_factor;
	double sweep_factor;
	size_t sweep_limit;
	size_t size;
	gc_allocation_t **allocs;
} gc_allocation_map_t;

typedef struct garbage_collector {
    struct gc_allocation_map *allocs; // allocation map
    // struct gc_allocation_map *roots; // root map
    bool paused; // (temporarily) switch gc on/off
    void *bos; // bottom of stack
} garbage_collector_t;

garbage_collector_t _gc;
garbage_collector_t *gc = &_gc;

static gc_allocation_t *gc_allocation_new(void *ptr, size_t size) {
	gc_allocation_t *a = (gc_allocation_t *)malloc(sizeof(gc_allocation_t));
	a->ptr = ptr;
	a->size = size;
	a->array_length = NULL;
	a->tag = GC_TAG_NONE;
	a->roots = 0;
	a->next = NULL;
	a->cut = NULL;
	return a;
}

static void gc_allocation_delete(gc_allocation_t *a) {
	free(a);
}

static gc_allocation_map_t *gc_allocation_map_new(size_t min_capacity, size_t capacity,
		double sweep_factor, double downsize_factor, double upsize_factor) {
	gc_allocation_map_t *am = (gc_allocation_map_t *)malloc(sizeof(gc_allocation_map_t));
	am->min_capacity = min_capacity;
	am->capacity = capacity;
	am->sweep_factor = sweep_factor;
	am->sweep_limit = (int)(sweep_factor * am->capacity);
	am->downsize_factor = downsize_factor;
	am->upsize_factor = upsize_factor;
	am->allocs = (gc_allocation_t **)calloc(am->capacity, sizeof(gc_allocation_t *));
	am->size = 0;
	return am;
}

static void gc_allocation_map_delete(gc_allocation_map_t *am) {
	// Iterate over the map
	gc_allocation_t *alloc;
	gc_allocation_t *tmp;
	for (size_t i = 0; i < am->capacity; ++i) {
		if ((alloc = am->allocs[i])) {
			// Make sure to follow the chain inside a bucket
			while (alloc) {
				tmp = alloc;
				alloc = alloc->next;
				// free the management structure
				gc_allocation_delete(tmp);
			}
		}
	}
	free(am->allocs);
	free(am);
}

static size_t gc_hash(void *ptr) {
	return ((uintptr_t)ptr) >> 3;
}

static void gc_allocation_map_resize(gc_allocation_map_t *am, size_t new_capacity) {
	if (new_capacity <= am->min_capacity) {
		return;
	}
	// Replaces the existing items array in the hash table
	// with a resized one and pushes items into the new, correct buckets
	gc_allocation_t **resized_allocs = calloc(new_capacity, sizeof(gc_allocation_t *));

	for (size_t i = 0; i < am->capacity; ++i) {
		gc_allocation_t *alloc = am->allocs[i];
		while (alloc) {
			gc_allocation_t *next_alloc = alloc->next;
			size_t new_index = gc_hash(alloc->ptr) & (new_capacity - 1);
			alloc->next = resized_allocs[new_index];
			resized_allocs[new_index] = alloc;
			alloc = next_alloc;
		}
	}
	free(am->allocs);
	am->capacity = new_capacity;
	am->allocs = resized_allocs;
	am->sweep_limit = am->size + am->sweep_factor * (am->capacity - am->size);
}

static bool gc_allocation_map_resize_to_fit(gc_allocation_map_t *am) {
	double load_factor = (double)am->size / (double)am->capacity;
	if (load_factor > am->upsize_factor) {
		gc_allocation_map_resize(am, am->capacity * 2);
		return true;
	}
	if (load_factor < am->downsize_factor) {
		gc_allocation_map_resize(am, am->capacity / 2);
		return true;
	}
	return false;
}

static gc_allocation_t *gc_allocation_map_get(gc_allocation_map_t *am, void *ptr) {
	size_t index = gc_hash(ptr) & (am->capacity - 1); // % am->capacity
	gc_allocation_t *cur = am->allocs[index];
	while(cur) {
		if (cur->ptr == ptr) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

static gc_allocation_t *gc_allocation_map_put(gc_allocation_map_t *am, gc_allocation_t *alloc) {
	size_t index = gc_hash(alloc->ptr) & (am->capacity - 1);
	gc_allocation_t *cur = am->allocs[index];
	gc_allocation_t *prev = NULL;
	/* Upsert if ptr is already known */
	while (cur != NULL) {
		if (cur->ptr == alloc->ptr) {
			// found it
			alloc->next = cur->next;
			if (!prev) {
				// position 0
				am->allocs[index] = alloc;
			}
			else {
				// in the list
				prev->next = alloc;
			}
			gc_allocation_delete(cur);
			return alloc;

		}
		prev = cur;
		cur = cur->next;
	}
	/* Insert at the front of the separate chaining list */
	cur = am->allocs[index];
	alloc->next = cur;
	am->allocs[index] = alloc;
	am->size++;
	void *p = alloc->ptr;
	if (gc_allocation_map_resize_to_fit(am)) {
		alloc = gc_allocation_map_get(am, p);
	}
	return alloc;
}

static void gc_allocation_map_remove(gc_allocation_map_t *am, void *ptr, bool allow_resize) {
	// ignores unknown keys
	size_t index = gc_hash(ptr) & (am->capacity - 1);
	gc_allocation_t *cur = am->allocs[index];
	gc_allocation_t *prev = NULL;
	gc_allocation_t *next;
	while (cur != NULL) {
		next = cur->next;
		if (cur->ptr == ptr) {
			// found it
			if (!prev) {
				// first item in list
				am->allocs[index] = cur->next;
			}
			else {
				// not the first item in the list
				prev->next = cur->next;
			}
			gc_allocation_delete(cur);
			am->size--;
		}
		else {
			// move on
			prev = cur;
		}
		cur = next;
	}
	if (allow_resize) {
		gc_allocation_map_resize_to_fit(am);
	}
}

static void* gc_mcalloc(size_t count, size_t size) {
	if (!count) {
		return malloc(size);
	}
	return calloc(count, size);
}

static void *gc_allocate(size_t count, size_t size) {
	/* Allocation logic that generalizes over malloc/calloc */
	/* Check if we reached the high-water mark and need to clean up */
	if (gc->allocs->size > gc->allocs->sweep_limit && !gc->paused) {
		size_t freed_mem = _gc_run();
	}
	/* With cleanup out of the way, attempt to allocate memory */
	void *ptr = gc_mcalloc(count, size);
	size_t alloc_size = count ? count * size : size;
	/* If allocation fails, force an out-of-policy run to free some memory and try again. */
	if (!ptr && !gc->paused) {
		_gc_run();
		ptr = gc_mcalloc(count, size);
	}
	/* Start managing the memory we received from the system */
	if (ptr) {
		gc_allocation_t *alloc = gc_allocation_map_put(gc->allocs, gc_allocation_new(ptr, alloc_size));
		/* Deal with metadata allocation failure */
		if (alloc) {
			ptr = alloc->ptr;
		}
		else {
			/* We failed to allocate the metadata, fail cleanly. */
			free(ptr);
			ptr = NULL;
		}
	}
	return ptr;
}

void _gc_array(void *ptr, int *length) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, ptr);
	if (alloc) {
		// alloc->array_length = length;
	}
}

void _gc_leaf(void *ptr) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, ptr);
	if (alloc) {
		alloc->tag |= GC_TAG_LEAF;
	}
}

void _gc_root(void *ptr) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, ptr);
	if (alloc) {
		alloc->roots++;
		alloc->tag |= GC_TAG_ROOT;
	}
}

void _gc_unroot(void *ptr) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, ptr);
	if (alloc) {
		alloc->roots--;
		if (alloc->roots == 0) {
			alloc->tag &= ~GC_TAG_ROOT;
		}
	}
}

static inline uint64_t pad(int di, int n) {
	return (n - (di % n)) % n;
}

// Cut out a leaf out of an existing allocation
void *_gc_cut(void *ptr, size_t pos, size_t size) {
	size += pad(size, 8);

	// Start
	gc_allocation_t *start = gc_allocation_map_get(gc->allocs, ptr);
	while (start->cut) {
		start = start->cut;
	}
	size_t start_size = start->size;
	pos -= start->ptr - ptr;
	start->size = pos;

	// Middle
	gc_allocation_t *middle = gc_allocation_new(start->ptr + pos, size);
	middle->tag |= GC_TAG_CUT;
	middle->tag |= GC_TAG_LEAF;
	start->cut = middle;
	gc_allocation_map_put(gc->allocs, middle);

	// End
	gc_allocation_t *end = gc_allocation_new(start->ptr + pos + size, start_size - size - pos);
	end->tag |= GC_TAG_CUT;
	middle->cut = end;
	gc_allocation_map_put(gc->allocs, end);

	return middle->ptr;
}

static void gc_mark_alloc(void *ptr) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, ptr);
	/* Mark if alloc exists and is not tagged already, otherwise skip */
	if (alloc && !(alloc->tag & GC_TAG_MARK)) {
		alloc->tag |= GC_TAG_MARK;
		if (!(alloc->tag & GC_TAG_LEAF)) { // Skip contents
			/* Iterate over allocation contents and mark them as well */
			int size = alloc->array_length ? (*alloc->array_length) * PTRSIZE : alloc->size;

			for (char *p = (char *)alloc->ptr; p <= (char *)alloc->ptr + size - PTRSIZE; ++p) {
				gc_mark_alloc(*(void **)p);
			}
		}
	}
}

static void gc_mark_stack() {
	void *tos = __builtin_frame_address(0);
	void *bos = gc->bos;
	/* The stack grows towards smaller memory addresses, hence we scan tos->bos.
	 * Stop scanning once the distance between tos & bos is too small to hold a valid pointer */
	for (char *p = (char *)tos; p <= (char *)bos - PTRSIZE; ++p) {
		gc_mark_alloc(*(void **)p);
	}
}

static void gc_mark_roots() {
	for (size_t i = 0; i < gc->allocs->capacity; ++i) {
		gc_allocation_t *chunk = gc->allocs->allocs[i];
		while (chunk) {
			if (chunk->tag & GC_TAG_ROOT) {
				gc_mark_alloc(chunk->ptr);
			}
			chunk = chunk->next;
		}
	}
}

static void gc_mark() {
	/* Note: We only look at the stack and the heap, and ignore BSS. */
	/* Scan the heap for roots */
	gc_mark_roots();
	/* Dump registers onto stack and scan the stack */
	void (*volatile _mark_stack)(void) = gc_mark_stack;
	jmp_buf ctx;
	memset(&ctx, 0, sizeof(jmp_buf));
	setjmp(ctx);
	_mark_stack();
}

static size_t gc_sweep() {
	size_t total = 0;
	for (size_t i = 0; i < gc->allocs->capacity; ++i) {
		gc_allocation_t *chunk = gc->allocs->allocs[i];
		gc_allocation_t *next = NULL;
		/* Iterate over separate chaining */
		while (chunk) {
			if (chunk->tag & GC_TAG_MARK) {
				/* unmark */
				chunk->tag &= ~GC_TAG_MARK;
				chunk = chunk->next;
			}
			else if (chunk->tag & GC_TAG_CUT) {
				chunk = chunk->next;
			}
			else {
				/* no reference to this chunk, hence delete it */
				total += chunk->size;
				free(chunk->ptr);
				/* and remove it from the bookkeeping */
				next = chunk->next;
				gc_allocation_t *cut = chunk->cut;
				gc_allocation_map_remove(gc->allocs, chunk->ptr, false);
				while (cut) {
					gc_allocation_t *next = cut->cut;
					gc_allocation_map_remove(gc->allocs, cut->ptr, false);
					cut = next;
				}
				chunk = next;
			}
		}
	}
	gc_allocation_map_resize_to_fit(gc->allocs);
	return total;
}

void *_gc_calloc(size_t count, size_t size) {
	return gc_allocate(count, size);
}

void *_gc_realloc(void *p, size_t size) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, p);
	if (p && !alloc) {
		// the user passed an unknown pointer
		return NULL;
	}
	void *q = realloc(p, size);
	if (!q) {
		// realloc failed but p is still valid
		return NULL;
	}
	if (!p) {
		// allocation, not reallocation
		gc_allocation_t *alloc = gc_allocation_map_put(gc->allocs, gc_allocation_new(q, size));
		return alloc->ptr;
	}
	if (p == q) {
		// successful reallocation w/o copy
		alloc->size = size;
	}
	else {
		// successful reallocation w/ copy
		gc_allocation_map_remove(gc->allocs, p, true);
		gc_allocation_map_put(gc->allocs, gc_allocation_new(q, size));
	}
	return q;
}

void _gc_free(void *ptr) {
	gc_allocation_t *alloc = gc_allocation_map_get(gc->allocs, ptr);
	if (alloc) {
		free(ptr);
		gc_allocation_map_remove(gc->allocs, ptr, true);
	}
}

void _gc_start(void *bos) {
	gc->paused = false;
	gc->bos = bos;
	// Create allocation map with no downsizing
	// Capacity must be a power of two
	gc->allocs = gc_allocation_map_new(1024 * 1024, 1024 * 1024, 0.5, 0.0, 0.8);
	// gc->roots = gc_allocation_map_new(1024, 1024, 0.5, 0.0, 0.8);
}

void _gc_pause() {
	gc->paused = true;
}

void _gc_resume() {
	gc->paused = false;
}

size_t _gc_stop() {
	size_t collected = gc_sweep();
	gc_allocation_map_delete(gc->allocs);
	return collected;
}

size_t _gc_run() {
	// double t = kinc_time();
	gc_mark();
	size_t collected = gc_sweep();
	// kinc_log(KINC_LOG_LEVEL_INFO, "gc took %fms, freed %db.\n", (kinc_time() - t) * 1000, collected);
	return collected;
}
