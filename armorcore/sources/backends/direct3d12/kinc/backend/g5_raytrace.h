#pragma once

#include "d3d12mini.h"

struct ID3D12StateObject;
struct ID3D12Resource;

typedef struct {
	struct ID3D12StateObject *dxr_state;
	struct ID3D12Resource *raygen_shader_table;
	struct ID3D12Resource *miss_shader_table;
	struct ID3D12Resource *hitgroup_shader_table;
} kinc_g5_raytrace_pipeline_impl_t;

typedef struct {
	struct ID3D12Resource *bottom_level_accel[16];
	struct ID3D12Resource *top_level_accel;
} kinc_g5_raytrace_acceleration_structure_impl_t;

typedef struct {
	struct ID3D12Resource *renderTarget;
	struct ID3D12Resource *renderTargetReadback;
	struct ID3D12DescriptorHeap *renderTargetDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDescriptorHeap;
	struct ID3D12DescriptorHeap *depthStencilDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDepthDescriptorHeap;
	struct ID3D12Resource *depthStencilTexture;
	struct D3D12Viewport viewport;
	struct D3D12Rect scissor;
	int stage;
	int stage_depth;
	int framebuffer_index;
} RenderTarget5Impl;
