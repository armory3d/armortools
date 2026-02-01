#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define ALIGNMENT   8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define HEAP_SIZE 1024 * 1024 * 512
static unsigned char heap[HEAP_SIZE] __attribute__((aligned(ALIGNMENT)));
static size_t        heap_top = ALIGNMENT;

typedef struct block {
	size_t        size;
	struct block *next;
} block_t;
static block_t *free_list = NULL;

#ifdef IRON_WASM
__attribute__((export_name("malloc")))
#endif
void *
malloc(size_t size) {
	size           = ALIGN(size);
	block_t **link = &free_list;
	block_t  *curr = free_list;
	while (curr) {
		if (curr->size >= size) {
			if (curr->size >= size + sizeof(block_t) + ALIGNMENT) {
				block_t *remainder = (block_t *)((unsigned char *)curr + sizeof(block_t) + size);
				remainder->size    = curr->size - size - sizeof(block_t);
				remainder->next    = curr->next;
				*link              = remainder;
				curr->size         = size;
			}
			else {
				*link = curr->next;
			}
			return (void *)(curr + 1);
		}
		link = &curr->next;
		curr = curr->next;
	}
	if (heap_top + sizeof(block_t) + size > HEAP_SIZE) {
		printf("malloc: out of memory");
		return NULL;
	}
	block_t *ptr = (block_t *)&heap[heap_top];
	ptr->size    = size;
	heap_top += sizeof(block_t) + size;
	return (void *)(ptr + 1);
}

void free(void *mem) {
	if (!mem) {
		return;
	}
	block_t *block = (block_t *)mem - 1;
	block_t *curr  = free_list;
	block_t *prev  = NULL;
	while (curr && curr < block) {
		prev = curr;
		curr = curr->next;
	}
	if (prev) {
		prev->next = block;
	}
	else {
		free_list = block;
	}
	block->next = curr;
	if (curr && (unsigned char *)(block + 1) + block->size == (unsigned char *)curr) {
		block->size += sizeof(block_t) + curr->size;
		block->next = curr->next;
	}
	if (prev && (unsigned char *)(prev + 1) + prev->size == (unsigned char *)block) {
		prev->size += sizeof(block_t) + block->size;
		prev->next = block->next;
	}
}

void *calloc(size_t num, size_t size) {
	size_t total = num * size;
	void  *ptr   = malloc(total);
	if (ptr) {
		memset(ptr, 0, total);
	}
	return ptr;
}

void *realloc(void *mem, size_t size) {
	if (!mem) {
		return malloc(size);
	}
	block_t *b = (block_t *)mem - 1;
	if (b->size >= size) {
		return mem;
	}
	void *ptr = malloc(size);
	if (ptr) {
		memcpy(ptr, mem, b->size);
		free(mem);
	}
	return ptr;
}

void *aligned_alloc(size_t align, size_t size) {
	heap_top = (heap_top + align - 1) & ~(align - 1);
	return malloc(size);
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

void *memmove(void *destination, const void *source, size_t num) {
	unsigned char *d = (unsigned char *)destination;
	unsigned char *s = (unsigned char *)source;
	if (d < s || d == s) {
		for (size_t i = 0; i < num; ++i) {
			d[i] = s[i];
		}
	}
	else {
		for (size_t i = num; i > 0; --i) {
			d[i - 1] = s[i - 1];
		}
	}
	return destination;
}
