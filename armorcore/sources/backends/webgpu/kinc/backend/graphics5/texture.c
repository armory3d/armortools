#include <kinc/graphics5/texture.h>

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {

	// WGPUExtent3D size = {};
	// size.width = kinc_width();
	// size.height = kinc_height();
	// size.depth = 1;

	// WGPUTextureDescriptor tDesc = {};
	// tDesc.sampleCount = 1;
	// tDesc.format = WGPUTextureFormat_BGRA8Unorm;
	// tDesc.usage = WGPUTextureUsage_OutputAttachment;
	// tDesc.size = size;
	// tDesc.dimension = WGPUTextureDimension_2D;
	// tDesc.mipLevelCount = 1;
	// tDesc.arrayLayerCount = 1;
	// WGPUTexture texture = wgpuDeviceCreateTexture(device, &tDesc);

	// WGPUTextureViewDescriptor tvDesc = {};
	// tvDesc.format = WGPUTextureFormat_BGRA8Unorm;
	// tvDesc.dimension = WGPUTextureViewDimension_2D;
 	// WGPUTextureView textureView = wgpuTextureCreateView(texture, &tvDesc);

}

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {}
void kinc_g5_texture_init_from_encoded_data(kinc_g5_texture_t *texture, void *data, int size, const char *format, bool readable) {}
void kinc_g5_texture_init_from_data(kinc_g5_texture_t *texture, void *data, int width, int height, int format, bool readable) {}
void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {}
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	return NULL;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *texture) {}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	return 32;
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_image_t *mipmap, int level) {}
