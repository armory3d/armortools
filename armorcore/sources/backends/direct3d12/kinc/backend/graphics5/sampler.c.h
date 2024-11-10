#include <kinc/graphics5/sampler.h>

static D3D12_TEXTURE_ADDRESS_MODE convert_texture_addressing(kinc_g5_texture_addressing_t addressing) {
	switch (addressing) {
	case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case KINC_G5_TEXTURE_ADDRESSING_BORDER:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	default:
		assert(false);
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}

static D3D12_FILTER convert_filter(kinc_g5_texture_filter_t minification, kinc_g5_texture_filter_t magnification, kinc_g5_mipmap_filter_t mipmap) {
	switch (minification) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		switch (magnification) {
		case KINC_G5_TEXTURE_FILTER_POINT:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_MAG_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_LINEAR:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
		}
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		switch (magnification) {
		case KINC_G5_TEXTURE_FILTER_POINT:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_LINEAR:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
		}
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return D3D12_FILTER_ANISOTROPIC;
	}

	assert(false);
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

static D3D12_FILTER convert_comparison_filter(kinc_g5_texture_filter_t minification, kinc_g5_texture_filter_t magnification, kinc_g5_mipmap_filter_t mipmap) {
	switch (minification) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		switch (magnification) {
		case KINC_G5_TEXTURE_FILTER_POINT:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_LINEAR:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		}
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		switch (magnification) {
		case KINC_G5_TEXTURE_FILTER_POINT:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_LINEAR:
			switch (mipmap) {
			case KINC_G5_MIPMAP_FILTER_NONE:
			case KINC_G5_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			case KINC_G5_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			}
		case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		}
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return D3D12_FILTER_COMPARISON_ANISOTROPIC;
	}

	assert(false);
	return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
}

void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options) {
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
	descHeapSampler.NumDescriptors = 2;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&descHeapSampler, IID_GRAPHICS_PPV_ARGS(&sampler->impl.sampler_heap));

	D3D12_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D12_SAMPLER_DESC));
	samplerDesc.Filter = options->is_comparison ? convert_comparison_filter(options->minification_filter, options->magnification_filter, options->mipmap_filter)
	                                            : convert_filter(options->minification_filter, options->magnification_filter, options->mipmap_filter);
	samplerDesc.AddressU = convert_texture_addressing(options->u_addressing);
	samplerDesc.AddressV = convert_texture_addressing(options->v_addressing);
	samplerDesc.AddressW = convert_texture_addressing(options->w_addressing);
	samplerDesc.MinLOD = options->lod_min_clamp;
	samplerDesc.MaxLOD = options->lod_max_clamp;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = options->max_anisotropy;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	device->CreateSampler(&samplerDesc, sampler->impl.sampler_heap->GetCPUDescriptorHandleForHeapStart());
}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {}
