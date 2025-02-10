#include <kinc/graphics5/sampler.h>

static VkCompareOp convert_compare_mode(kinc_g5_compare_mode_t compare);

static VkSamplerAddressMode convert_addressing(kinc_g5_texture_addressing_t mode) {
	switch (mode) {
	case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case KINC_G5_TEXTURE_ADDRESSING_BORDER:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	default:
		assert(false);
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

static VkSamplerMipmapMode convert_mipmap_mode(kinc_g5_mipmap_filter_t filter) {
	switch (filter) {
	case KINC_G5_MIPMAP_FILTER_NONE:
	case KINC_G5_MIPMAP_FILTER_POINT:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case KINC_G5_MIPMAP_FILTER_LINEAR:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	default:
		assert(false);
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
}

static VkFilter convert_texture_filter(kinc_g5_texture_filter_t filter) {
	switch (filter) {
	case KINC_G5_TEXTURE_FILTER_POINT:
		return VK_FILTER_NEAREST;
	case KINC_G5_TEXTURE_FILTER_LINEAR:
		return VK_FILTER_LINEAR;
	case KINC_G5_TEXTURE_FILTER_ANISOTROPIC:
		return VK_FILTER_LINEAR; // ?
	default:
		assert(false);
		return VK_FILTER_NEAREST;
	}
}

void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options) {
	VkSamplerCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = NULL;
	info.flags = 0;

	info.addressModeU = convert_addressing(options->u_addressing);
	info.addressModeV = convert_addressing(options->v_addressing);
	info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	info.mipmapMode = convert_mipmap_mode(options->mipmap_filter);

	info.magFilter = convert_texture_filter(options->magnification_filter);
	info.minFilter = convert_texture_filter(options->minification_filter);

	info.anisotropyEnable =
	    (options->magnification_filter == KINC_G5_TEXTURE_FILTER_ANISOTROPIC || options->minification_filter == KINC_G5_TEXTURE_FILTER_ANISOTROPIC);
	info.maxAnisotropy = 1;

	info.maxLod = 32;
	info.minLod = 0;

	vkCreateSampler(vk_ctx.device, &info, NULL, &sampler->impl.sampler);
}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {
	vkDestroySampler(vk_ctx.device, sampler->impl.sampler, NULL);
}
