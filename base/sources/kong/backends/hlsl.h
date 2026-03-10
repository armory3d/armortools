#ifndef KONG_BACKENDS_HLSL_HEADER
#define KONG_BACKENDS_HLSL_HEADER

#include "../api.h"

#include <stdbool.h>
#include <stdint.h>

void hlsl_export(char *directory, api_kind d3d, bool debug);

#endif
