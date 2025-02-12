#include "shaderhash.h"

// djb2
uint32_t kinc_internal_hash_name(unsigned char *str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = hash * 33 ^ c;
	}
	return hash;
}
