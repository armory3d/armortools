#include <iron_gpu.h>

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

	texture->_uploaded = true;
	texture->format = format;

}

void kinc_g5_texture_init_from_bytes(kinc_g5_texture_t *texture, void *data, int width, int height, kinc_image_format_t format) {}
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

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level) {}

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
    target->width = target->width = width;
	target->height = target->height = height;
	target->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
}

void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *renderTarget) {}

void kinc_g5_render_target_set_depth_from(kinc_g5_render_target_t *renderTarget, kinc_g5_render_target_t *source) {}

void kinc_g5_render_target_get_pixels(kinc_g5_render_target_t *renderTarget, uint8_t *data) {}
