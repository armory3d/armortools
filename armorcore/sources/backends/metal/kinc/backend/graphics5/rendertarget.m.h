#include <kinc/graphics5/rendertarget.h>

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/rendertarget.h>

#import <Metal/Metal.h>

id getMetalDevice(void);
id getMetalEncoder(void);

static MTLPixelFormat convert_format(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return MTLPixelFormatRGBA32Float;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return MTLPixelFormatRGBA16Float;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return MTLPixelFormatR32Float;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return MTLPixelFormatR16Float;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return MTLPixelFormatR8Unorm;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
	default:
		return MTLPixelFormatBGRA8Unorm;
	}
}

static void render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits,
                               int stencilBufferBits, int samples_per_pixel, int framebuffer_index) {
	memset(target, 0, sizeof(kinc_g5_render_target_t));

	target->texWidth = width;
	target->texHeight = height;

	target->framebuffer_index = framebuffer_index;

	id<MTLDevice> device = getMetalDevice();

	MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = convert_format(format);
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
	descriptor.resourceOptions = MTLResourceStorageModePrivate;

	target->impl._tex = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];

	if (depthBufferBits > 0) {
		MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor new];
		depthDescriptor.textureType = MTLTextureType2D;
		depthDescriptor.width = width;
		depthDescriptor.height = height;
		depthDescriptor.depth = 1;
		depthDescriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
		depthDescriptor.arrayLength = 1;
		depthDescriptor.mipmapLevelCount = 1;
		depthDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
		depthDescriptor.resourceOptions = MTLResourceStorageModePrivate;

		target->impl._depthTex = (__bridge_retained void *)[device newTextureWithDescriptor:depthDescriptor];
	}

	target->impl._texReadback = NULL;
}

void kinc_g5_render_target_init_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                   int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	render_target_init(target, width, height, format, depthBufferBits, stencilBufferBits, samples_per_pixel, -1);
}

static int framebuffer_count = 0;

void kinc_g5_render_target_init_framebuffer_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                               int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	render_target_init(target, width, height, format, depthBufferBits, stencilBufferBits, samples_per_pixel, framebuffer_count);
	framebuffer_count += 1;
}

void kinc_g5_render_target_init_cube_with_multisampling(kinc_g5_render_target_t *target, int cubeMapSize, kinc_g5_render_target_format_t format,
                                                        int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	target->impl._tex = NULL;
	target->impl._depthTex = NULL;
	target->impl._texReadback = NULL;
}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target) {
	id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)target->impl._tex;
	tex = nil;
	target->impl._tex = NULL;

	id<MTLTexture> depthTex = (__bridge_transfer id<MTLTexture>)target->impl._depthTex;
	depthTex = nil;
	target->impl._depthTex = NULL;

	id<MTLTexture> texReadback = (__bridge_transfer id<MTLTexture>)target->impl._texReadback;
	texReadback = nil;
	target->impl._texReadback = NULL;

	if (target->framebuffer_index >= 0) {
		framebuffer_count -= 1;
	}
}

#if 0
void kinc_g5_set_render_target_descriptor(kinc_g5_render_target_t *renderTarget, kinc_g5_texture_descriptor_t descriptor) {
    MTLSamplerDescriptor* desc = (MTLSamplerDescriptor*) renderTarget->impl._samplerDesc;
    switch(descriptor.filter_minification) {
        case KINC_G5_TEXTURE_FILTER_POINT:
            desc.minFilter = MTLSamplerMinMagFilterNearest;
            break;
        default:
            desc.minFilter = MTLSamplerMinMagFilterLinear;
    }

    switch(descriptor.filter_magnification) {
        case KINC_G5_TEXTURE_FILTER_POINT:
            desc.magFilter = MTLSamplerMinMagFilterNearest;
            break;
        default:
            desc.minFilter = MTLSamplerMinMagFilterLinear;
    }

    switch(descriptor.addressing_u) {
        case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
            desc.sAddressMode = MTLSamplerAddressModeRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
            desc.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
            desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_BORDER:
            desc.sAddressMode = MTLSamplerAddressModeClampToBorderColor;
            break;
    }

    switch(descriptor.addressing_v) {
        case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
            desc.tAddressMode = MTLSamplerAddressModeRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
            desc.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
            desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_BORDER:
            desc.tAddressMode = MTLSamplerAddressModeClampToBorderColor;
            break;
    }
    id<MTLDevice> device = getMetalDevice();
    renderTarget->impl._sampler = [device newSamplerStateWithDescriptor:desc];
}
#endif

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source) {
	target->impl._depthTex = source->impl._depthTex;
}
