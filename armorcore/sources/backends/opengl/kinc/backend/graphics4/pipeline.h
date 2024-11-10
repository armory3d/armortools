#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned programId;
	char **textures;
	int *textureValues;
	int textureCount;
} kinc_g4_pipeline_impl_t;

#ifdef __cplusplus
}
#endif
