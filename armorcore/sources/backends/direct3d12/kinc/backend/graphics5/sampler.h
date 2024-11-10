#pragma once

struct ID3D12DescriptorHeap;

typedef struct kinc_g5_sampler_impl {
	struct ID3D12DescriptorHeap *sampler_heap;
} kinc_g5_sampler_impl_t;
