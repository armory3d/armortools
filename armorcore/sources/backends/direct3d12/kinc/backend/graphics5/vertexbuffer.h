#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
