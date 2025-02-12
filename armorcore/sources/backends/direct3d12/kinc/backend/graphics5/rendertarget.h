#pragma once

#include "d3d12mini.h"

enum RenderTargetResourceState { RenderTargetResourceStateUndefined, RenderTargetResourceStateRenderTarget, RenderTargetResourceStateTexture };

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
