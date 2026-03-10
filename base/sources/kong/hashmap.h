#ifndef KONG_HASH_MAP_HEADER
#define KONG_HASH_MAP_HEADER

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// based on https://valkey.io/blog/new-hash-table/

struct meta {
	uint8_t c        : 1;
	uint8_t presence : 7;
	uint8_t hashes[7];
};

struct container {
	int key;
};

struct bucket {
	struct meta meta;
	void       *entries[7];
};

#define HASH_MAP_SIZE 256

struct hash_map {
	struct bucket buckets[HASH_MAP_SIZE];
};

static inline uint64_t hash(int key) {
	uint64_t primary   = key % HASH_MAP_SIZE;
	uint8_t  secondary = key % HASH_MAP_SIZE;
	return primary | ((uint64_t)secondary << 56);
}

static inline struct bucket *allocate_bucket(void) {
#ifdef _WIN32
	struct bucket *new_bucket = (struct bucket *)_aligned_malloc(sizeof(struct bucket), 64);
#else
	struct bucket *new_bucket = (struct bucket *)aligned_alloc(64, sizeof(struct bucket));
#endif
	assert(new_bucket != NULL);
	memset(new_bucket, 0, sizeof(struct bucket));

	// cache line check
	assert((uint64_t)new_bucket % 64 == 0);
	assert(sizeof(struct bucket) == 64);

	return new_bucket;
}

static inline struct hash_map *hash_map_create(void) {
#ifdef _WIN32
	struct hash_map *map = _aligned_malloc(sizeof(struct hash_map), 64);
#else
	struct hash_map *map = aligned_alloc(64, sizeof(struct hash_map));
#endif
	assert(map != NULL);
	memset(map, 0, sizeof(struct hash_map));

	// cache line check
	assert((uint64_t)map % 64 == 0);

	return map;
}

static inline void hash_map_destroy(struct hash_map *map) {}

static inline void hash_map_add_to_bucket(struct bucket *bucket, struct container *value, uint8_t secondary_hash) {
	for (uint32_t index = 0; index < 7; ++index) {
		if (((bucket->meta.presence >> index) & 1) == 0) {
			bucket->entries[index]     = value;
			bucket->meta.hashes[index] = secondary_hash;

			bucket->meta.presence |= (1 << index);

			return;
		}
	}

	if (bucket->meta.c) {
		hash_map_add_to_bucket((struct bucket *)bucket->entries[6], value, secondary_hash);
	}
	else {
		bucket->meta.c = 1;

		struct container *previous                = (struct container *)bucket->entries[6];
		uint8_t           previous_secondary_hash = bucket->meta.hashes[6];

		struct bucket *new_bucket = allocate_bucket();

		bucket->entries[6] = new_bucket;

		hash_map_add_to_bucket(new_bucket, previous, previous_secondary_hash);
		hash_map_add_to_bucket(new_bucket, value, secondary_hash);
	}
}

static inline void hash_map_add(struct hash_map *map, struct container *value) {
	uint64_t hash_value = hash(value->key);
	uint64_t primary    = hash_value & 0xffffffffffffffu;
	uint8_t  secondary  = (uint8_t)(hash_value << 56);

	hash_map_add_to_bucket(&map->buckets[primary], value, secondary);
}

static inline struct container *hash_map_get_from_bucket(struct bucket *bucket, int key, uint8_t secondary_hash) {
	for (uint32_t index = 0; index < 6; ++index) {
		if (((bucket->meta.presence >> index) & 1) != 0 && bucket->meta.hashes[index] == secondary_hash) {
			struct container *value = (struct container *)bucket->entries[index];

			if (value->key == key) {
				return value;
			}
		}
	}

	if (bucket->meta.c) {
		struct bucket *next_bucket = (struct bucket *)bucket->entries[6];
		return hash_map_get_from_bucket(next_bucket, key, secondary_hash);
	}
	else {
		if (((bucket->meta.presence >> 6) & 1) != 0 && bucket->meta.hashes[6] == secondary_hash) {
			struct container *value = (struct container *)bucket->entries[6];

			if (value->key == key) {
				return value;
			}
		}
	}

	return NULL;
}

static inline struct container *hash_map_get(struct hash_map *map, int key) {
	uint64_t hash_value = hash(key);
	uint64_t primary    = hash_value & 0xffffffffffffffu;
	uint8_t  secondary  = (uint8_t)(hash_value << 56);

	struct bucket *bucket = &map->buckets[primary];

	return hash_map_get_from_bucket(bucket, key, secondary);
}

static inline void hash_map_iterate_in_bucket(struct bucket *bucket, void (*callback)(struct container *, void *data), void *data) {
	for (uint32_t index = 0; index < 6; ++index) {
		if (((bucket->meta.presence >> index) & 1) != 0) {
			struct container *value = (struct container *)bucket->entries[index];
			callback(value, data);
		}
	}

	if (bucket->meta.c) {
		struct bucket *next_bucket = (struct bucket *)bucket->entries[6];
		hash_map_iterate_in_bucket(next_bucket, callback, data);
	}
	else {
		if (((bucket->meta.presence >> 6) & 1) != 0) {
			struct container *value = (struct container *)bucket->entries[6];
			callback(value, data);
		}
	}
}

static inline void hash_map_iterate(struct hash_map *map, void (*callback)(struct container *, void *data), void *data) {
	for (uint32_t bucket_index = 0; bucket_index < HASH_MAP_SIZE; ++bucket_index) {
		struct bucket *bucket = &map->buckets[bucket_index];
		hash_map_iterate_in_bucket(bucket, callback, data);
	}
}

#endif
