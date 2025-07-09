#pragma once

#include <stdbool.h>
#include <stdint.h>

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12PipelineState;
struct ID3D12StateObject;
enum D3D12_RESOURCE_STATES;

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

struct D3D12VertexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	unsigned int StrideInBytes;
};

struct D3D12IndexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	int Format;
};

typedef struct {
	struct ID3D12PipelineState *pso;
} gpu_pipeline_impl_t;

typedef struct {
	void *shader;
	uint8_t *data;
	int length;
} gpu_shader_impl_t;

typedef struct {
	struct ID3D12Resource *image;
	struct ID3D12Resource *readback;
	struct ID3D12DescriptorHeap *srv_descriptor_heap;
	struct ID3D12DescriptorHeap *rtv_descriptor_heap;
	int stride;
} gpu_texture_impl_t;

typedef struct {
	struct ID3D12Resource *buffer;
	struct D3D12VertexBufferView vertex_buffer_view;
	struct D3D12IndexBufferView index_buffer_view;
	int stride;
	int last_start;
	int last_count;
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
