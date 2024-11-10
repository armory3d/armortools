#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Resource;

typedef struct {
	struct ID3D12Resource *constant_buffer;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

extern bool kinc_g5_transposeMat3;
extern bool kinc_g5_transposeMat4;

#ifdef __cplusplus
}
#endif
