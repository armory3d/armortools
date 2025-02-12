#pragma once

#include <stdbool.h>

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain;

typedef void *HANDLE;
typedef unsigned __int64 UINT64;

struct D3D12Viewport {
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;
};

struct D3D12Rect {
	long left;
	long top;
	long right;
	long bottom;
};

#define QUEUE_SLOT_COUNT 2

struct dx_window {
	struct IDXGISwapChain *swapChain;
	UINT64 current_fence_value;
	UINT64 fence_values[QUEUE_SLOT_COUNT];
	HANDLE frame_fence_events[QUEUE_SLOT_COUNT];
	struct ID3D12Fence *frame_fences[QUEUE_SLOT_COUNT];
	int width;
	int height;
	int new_width;
	int new_height;
	int current_backbuffer;
	bool vsync;
	int window_index;
};
