#pragma once

#include <stdbool.h>
#include <stdint.h>

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
};

struct kinc_g5_pipeline;
struct kinc_g5_texture;
struct kinc_g5_sampler;

#define KINC_INTERNAL_G5_TEXTURE_COUNT 16

typedef struct {
	struct ID3D12CommandAllocator *_commandAllocator;
	struct ID3D12GraphicsCommandList *_commandList;
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;
	bool open;

	struct D3D12Rect current_full_scissor;

	uint64_t fence_value;
	struct ID3D12Fence *fence;
	HANDLE fence_event;

	struct kinc_g5_texture *currentTextures[KINC_INTERNAL_G5_TEXTURE_COUNT];
	struct kinc_g5_sampler *current_samplers[KINC_INTERNAL_G5_TEXTURE_COUNT];

	int heapIndex;
	struct ID3D12DescriptorHeap *srvHeap;
	struct ID3D12DescriptorHeap *samplerHeap;
} CommandList5Impl;

typedef struct {
	uint32_t hash;
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_compute_internal_shader_constant_t;

typedef struct {
	uint32_t hash;
	uint32_t index;
} kinc_internal_hash_index_t;

typedef struct kinc_g5_compute_shader_impl {
	kinc_compute_internal_shader_constant_t constants[64];
	int constantsSize;
	kinc_internal_hash_index_t attributes[64];
	kinc_internal_hash_index_t textures[64];
	uint8_t *data;
	int length;
	struct ID3D12Buffer *constantBuffer;
	struct ID3D12PipelineState *pso;
} kinc_g5_compute_shader_impl;

struct kinc_g5_shader;

struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;

typedef struct {
	struct ID3D12PipelineState *pso;
	int textures;
} PipelineState5Impl;

typedef struct {
	struct ID3D12PipelineState *pso;
	int textures;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	uint32_t vertexSize;
	int fragmentOffset;
	uint32_t fragmentSize;
	int computeOffset;
	uint32_t computeSize;
	int geometryOffset;
	uint32_t geometrySize;
	int tessEvalOffset;
	uint32_t tessEvalSize;
	int tessControlOffset;
	uint32_t tessControlSize;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;

struct kinc_g5_pipeline;
struct kinc_g5_command_list;

void kinc_g5_internal_setConstants(struct kinc_g5_command_list *commandList, struct kinc_g5_pipeline *pipeline);

typedef struct kinc_g5_sampler_impl {
	struct ID3D12DescriptorHeap *sampler_heap;
} kinc_g5_sampler_impl_t;

typedef struct {
	char name[64];
	uint32_t offset;
	uint32_t size;
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
	int constantsSize;
	ShaderAttribute attributes[32];
	ShaderTexture textures[32];
	int texturesCount;
	void *shader;
	uint8_t *data;
	int length;
} Shader5Impl;

uint32_t kinc_internal_hash_name(unsigned char *str);

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

typedef struct {
	int unit;
} TextureUnit5Impl;

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
	// struct ID3D12DescriptorHeap *srvDescriptorHeap;
	struct ID3D12DescriptorHeap *depthStencilDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDepthDescriptorHeap;
	struct ID3D12Resource *depthStencilTexture;
	struct D3D12Viewport viewport;
	struct D3D12Rect scissor;
	// int stage;
	int stage_depth;
	int framebuffer_index;
} Texture5Impl;

struct kinc_g5_texture;
struct kinc_g5_command_list;

void kinc_g5_internal_set_textures(struct kinc_g5_command_list *commandList);
void kinc_g5_internal_texture_set(struct kinc_g5_command_list *commandList, struct kinc_g5_texture *texture, int unit);

struct ID3D12Resource;

struct D3D12VertexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	unsigned int StrideInBytes;
};

typedef struct {
	struct ID3D12Resource *uploadBuffer;
	struct D3D12VertexBufferView view;

	int myCount;
	int myStride;
	int lastStart;
	int lastCount;
} VertexBuffer5Impl;

typedef struct {
	struct ID3D12Resource *constant_buffer;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

struct D3D12IindexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	int Format;
};

typedef struct {
	struct ID3D12Resource *index_buffer;
	struct D3D12IindexBufferView index_buffer_view;
	struct ID3D12Resource *upload_buffer;
	int count;
	bool gpu_memory;
	int last_start;
	int last_count;
} IndexBuffer5Impl;

struct kinc_g5_index_buffer;

void kinc_g5_internal_index_buffer_upload(struct kinc_g5_index_buffer *buffer, struct ID3D12GraphicsCommandList *commandList);

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
