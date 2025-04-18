#pragma once

#include <stdbool.h>
#include <stdint.h>

#define QUEUE_SLOT_COUNT 2
#define IRON_INTERNAL_G5_TEXTURE_COUNT 16

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain;
struct ID3D12PipelineState;
struct ID3D12RootSignature;
struct ID3D12StateObject;
struct iron_gpu_pipeline;
struct iron_gpu_texture;
struct iron_gpu_sampler;
struct iron_gpu_shader;
struct iron_gpu_command_list;
struct iron_gpu_buffer;

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
};

typedef struct {
	struct ID3D12CommandAllocator *_commandAllocator;
	struct ID3D12GraphicsCommandList *_commandList;
	struct iron_gpu_pipeline *_currentPipeline;
	int _indexCount;

	struct D3D12Rect current_full_scissor;

	uint64_t fence_value;
	struct ID3D12Fence *fence;
	HANDLE fence_event;

	struct iron_gpu_texture *currentTextures[IRON_INTERNAL_G5_TEXTURE_COUNT];
	struct iron_gpu_sampler *current_samplers[IRON_INTERNAL_G5_TEXTURE_COUNT];

	int heapIndex;
	struct ID3D12DescriptorHeap *srvHeap;
	struct ID3D12DescriptorHeap *samplerHeap;
} gpu_command_list_impl_t;

typedef struct {
	struct ID3D12PipelineState *pso;
	int textures;
} gpu_pipeline_impl_t;

typedef struct {
	int vertexOffset;
} gpu_constant_location_impl_t;

typedef struct iron_gpu_sampler_impl {
	struct ID3D12DescriptorHeap *sampler_heap;
} gpu_sampler_impl_t;

typedef struct {
	char name[64];
	uint32_t offset;
} ShaderConstant;

typedef struct {
	char name[64];
	int attribute;
} ShaderAttribute;

typedef struct {
	char name[64];
	int texture;
} ShaderTexture;

typedef struct {
	ShaderConstant constants[32];
	ShaderAttribute attributes[32];
	ShaderTexture textures[32];
	int texturesCount;
	void *shader;
	uint8_t *data;
	int length;
} gpu_shader_impl_t;

typedef struct {
	bool mipmap;
	int stage;
	int stride;
	struct ID3D12Resource *image;
	struct ID3D12Resource *uploadImage;
	struct ID3D12DescriptorHeap *srvDescriptorHeap;

	struct ID3D12Resource *renderTarget;
	struct ID3D12Resource *renderTargetReadback;
	struct ID3D12DescriptorHeap *renderTargetDescriptorHeap;
	struct ID3D12DescriptorHeap *depthStencilDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDepthDescriptorHeap;
	struct ID3D12Resource *depthStencilTexture;
	struct D3D12Viewport viewport;
	struct D3D12Rect scissor;

	int stage_depth;
	int framebuffer_index;
} gpu_texture_impl_t;

struct D3D12VertexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	unsigned int StrideInBytes;
};

struct D3D12IindexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	int Format;
};

typedef struct {
	struct ID3D12Resource *uploadBuffer;
	struct D3D12VertexBufferView view;
	int myCount;
	int myStride;
	int lastStart;
	int lastCount;

	struct ID3D12Resource *index_buffer;
	struct D3D12IindexBufferView index_buffer_view;
	struct ID3D12Resource *upload_buffer;
	int count;
	bool gpu_memory;
	int last_start;
	int last_count;

	struct ID3D12Resource *constant_buffer;
	int mySize;
} gpu_buffer_impl_t;

typedef struct {
	struct ID3D12StateObject *dxr_state;
	struct ID3D12Resource *raygen_shader_table;
	struct ID3D12Resource *miss_shader_table;
	struct ID3D12Resource *hitgroup_shader_table;
} gpu_raytrace_pipeline_impl_t;

typedef struct {
	struct ID3D12Resource *bottom_level_accel[16];
	struct ID3D12Resource *top_level_accel;
} gpu_raytrace_acceleration_structure_impl_t;
