#pragma once

#include <stdbool.h>
#include <stdint.h>

#define IRON_INTERNAL_G5_TEXTURE_COUNT 16

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12PipelineState;
struct ID3D12StateObject;

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

typedef struct {
	struct ID3D12PipelineState *pso;
} gpu_pipeline_impl_t;

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
	void *shader;
	uint8_t *data;
	int length;
} gpu_shader_impl_t;

typedef struct {
	bool mipmap;
	int stage;
	int stage_depth;
	int stride;
	struct ID3D12Resource *image;
	struct ID3D12Resource *uploadImage;
	struct ID3D12DescriptorHeap *srvDescriptorHeap;
	struct ID3D12Resource *renderTarget;
	struct ID3D12Resource *renderTargetReadback;
	struct ID3D12DescriptorHeap *renderTargetDescriptorHeap;
	struct ID3D12DescriptorHeap *depthDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDepthDescriptorHeap;
	struct ID3D12Resource *depthTexture;
	struct D3D12Viewport viewport;
	struct D3D12Rect scissor;
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
