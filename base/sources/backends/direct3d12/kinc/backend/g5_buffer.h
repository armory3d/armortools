#pragma once

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
