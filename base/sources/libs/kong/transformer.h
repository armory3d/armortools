#ifndef KONG_TRANSFORMER_HEADER
#define KONG_TRANSFORMER_HEADER

#include <stdint.h>

#define TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE 1

void transform(uint32_t flags);

#endif
