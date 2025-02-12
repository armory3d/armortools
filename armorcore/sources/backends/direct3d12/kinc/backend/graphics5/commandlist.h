#pragma once

#include <stdint.h>
#include "d3d12mini.h"

struct kinc_g5_pipeline;
struct kinc_g5_render_target;
struct kinc_g5_texture;
struct kinc_g5_sampler;

#define KINC_INTERNAL_G5_TEXTURE_COUNT 16

typedef struct {
	struct ID3D12CommandAllocator *_commandAllocator;
	struct ID3D12GraphicsCommandList *_commandList;
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;

#ifndef NDEBUG
	bool open;
#endif

	struct D3D12Rect current_full_scissor;

	// keep track of when a command-list is done
	uint64_t fence_value;
	struct ID3D12Fence *fence;
	HANDLE fence_event;

	struct kinc_g5_render_target *currentRenderTargets[KINC_INTERNAL_G5_TEXTURE_COUNT];
	struct kinc_g5_texture *currentTextures[KINC_INTERNAL_G5_TEXTURE_COUNT];
	struct kinc_g5_sampler *current_samplers[KINC_INTERNAL_G5_TEXTURE_COUNT];

	int heapIndex;
	struct ID3D12DescriptorHeap *srvHeap;
	struct ID3D12DescriptorHeap *samplerHeap;
} CommandList5Impl;
