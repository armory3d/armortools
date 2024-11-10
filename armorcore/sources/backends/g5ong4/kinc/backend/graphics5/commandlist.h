#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_pipeline;

#define KINC_G5ONG4_COMMANDS_SIZE 1024

typedef struct {
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;
	char commands[KINC_G5ONG4_COMMANDS_SIZE];
	int commandIndex;
	bool closed;
} CommandList5Impl;

#ifdef __cplusplus
}
#endif
