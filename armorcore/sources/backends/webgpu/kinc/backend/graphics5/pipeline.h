#pragma once

#include <webgpu/webgpu.h>

typedef struct {
	WGPURenderPipeline pipeline;
} PipelineState5Impl;

typedef struct {
	WGPUComputePipeline pipeline;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
	int computeOffset;
} ConstantLocation5Impl;
