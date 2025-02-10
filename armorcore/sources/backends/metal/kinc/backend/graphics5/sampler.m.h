#include <kinc/graphics5/sampler.h>

static MTLSamplerAddressMode convert_addressing(kinc_g5_texture_addressing_t mode) {
	switch (mode) {
	case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
		return MTLSamplerAddressModeRepeat;
	case KINC_G5_TEXTURE_ADDRESSING_BORDER:
		return MTLSamplerAddressModeClampToBorderColor;
	case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
		return MTLSamplerAddressModeClampToEdge;
	case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
		return MTLSamplerAddressModeMirrorRepeat;
	default:
		assert(false);
		return MTLSamplerAddressModeRepeat;
	}
}

static MTLSamplerMipFilter convert_mipmap_mode(kinc_g5_mipmap_filter_t filter) {
	switch (filter) {
	case KINC_G5_MIPMAP_FILTER_NONE:
		return MTLSamplerMipFilterNotMipmapped;
	case KINC_G5_MIPMAP_FILTER_POINT:
		return MTLSamplerMipFilterNearest;
	case KINC_G5_MIPMAP_FILTER_LINEAR:
		return MTLSamplerMipFilterLinear;
	default:
		assert(false);
		return MTLSamplerMipFilterNearest;
	}
}

static MTLSamplerMinMagFilter convert_texture_filter(kinc_g5_texture_filter_t filter) {
	switch (filter) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		return MTLSamplerMinMagFilterNearest;
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		return MTLSamplerMinMagFilterLinear;
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return MTLSamplerMinMagFilterLinear; // ?
	default:
		assert(false);
		return MTLSamplerMinMagFilterNearest;
	}
}

void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options) {
	id<MTLDevice> device = getMetalDevice();

	MTLSamplerDescriptor *desc = (MTLSamplerDescriptor *)[[MTLSamplerDescriptor alloc] init];
	desc.minFilter = convert_texture_filter(options->minification_filter);
	desc.magFilter = convert_texture_filter(options->magnification_filter);
	desc.sAddressMode = convert_addressing(options->u_addressing);
	desc.tAddressMode = convert_addressing(options->v_addressing);
	desc.mipFilter = convert_mipmap_mode(options->mipmap_filter);
	desc.maxAnisotropy = 1;
	desc.normalizedCoordinates = YES;
	desc.lodMinClamp = 0;
	desc.lodMaxClamp = 32;

	sampler->impl.sampler = (__bridge_retained void *)[device newSamplerStateWithDescriptor:desc];
}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {
	id<MTLSamplerState> mtl_sampler = (__bridge_transfer id<MTLSamplerState>)sampler->impl.sampler;
	mtl_sampler = nil;
}
