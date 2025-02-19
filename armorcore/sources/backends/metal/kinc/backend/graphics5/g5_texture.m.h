#include <kinc/g5_texture.h>
#include <kinc/g5.h>
#include <kinc/log.h>
#import <Metal/Metal.h>

id getMetalDevice(void);
id getMetalEncoder(void);

static MTLPixelFormat convert_image_format(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA32:
		return MTLPixelFormatRGBA8Unorm;
	case KINC_IMAGE_FORMAT_R8:
		return MTLPixelFormatR8Unorm;
	case KINC_IMAGE_FORMAT_R16:
		return MTLPixelFormatR16Float;
	case KINC_IMAGE_FORMAT_R32:
		return MTLPixelFormatR32Float;
	case KINC_IMAGE_FORMAT_RGBA128:
		return MTLPixelFormatRGBA32Float;
	case KINC_IMAGE_FORMAT_RGBA64:
		return MTLPixelFormatRGBA16Float;
	case KINC_IMAGE_FORMAT_BGRA32:
		return MTLPixelFormatBGRA8Unorm;
	}
}

static int formatByteSize(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_R16:
		return 2;
	case KINC_IMAGE_FORMAT_R8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_R32:
	default:
		return 4;
	}
}

static void create(kinc_g5_texture_t *texture, int width, int height, int format, bool writable) {
	texture->impl.has_mipmaps = false;
	id<MTLDevice> device = getMetalDevice();

	MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert_image_format((kinc_image_format_t)format)
	                                                                                      width:width
	                                                                                     height:height
	                                                                                  mipmapped:NO];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = convert_image_format((kinc_image_format_t)format);
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	// TODO: Make less textures writable
	if (writable) {
		descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
	}

	texture->impl._tex = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];
}

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->impl.data = malloc(width * height * (format == KINC_IMAGE_FORMAT_R8 ? 1 : 4));
	create(texture, width, height, format, true);
	texture->_uploaded = true;
}

void kinc_g5_texture_init_from_bytes(kinc_g5_texture_t *texture, void *data, int width, int height, kinc_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->data = data;
	texture->_uploaded = false;
	texture->impl.data = NULL;
	create(texture, width, height, format, true);
	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._tex;
	[tex replaceRegion:MTLRegionMake2D(0, 0, texture->width, texture->height)
	       mipmapLevel:0
	             slice:0
	         withBytes:data
	       bytesPerRow:kinc_g5_texture_stride(texture)
	     bytesPerImage:kinc_g5_texture_stride(texture) * texture->height];
}

void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->impl.data = malloc(width * height * (format == KINC_IMAGE_FORMAT_R8 ? 1 : 4));
	create(texture, width, height, format, true);
}

void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {
	id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)texture->impl._tex;
	tex = nil;
	texture->impl._tex = NULL;

	if (texture->impl.data != NULL) {
		free(texture->impl.data);
		texture->impl.data = NULL;
	}
}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	switch (texture->format) {
	case KINC_IMAGE_FORMAT_R8:
		return texture->width;
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_BGRA32:
		return texture->width * 4;
	case KINC_IMAGE_FORMAT_RGBA64:
		return texture->width * 8;
	case KINC_IMAGE_FORMAT_RGBA128:
		return texture->width * 16;
	}
}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	return (uint8_t *)texture->impl.data;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *tex) {
	id<MTLTexture> texture = (__bridge id<MTLTexture>)tex->impl._tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, tex->width, tex->height)
	           mipmapLevel:0
	                 slice:0
	             withBytes:tex->impl.data
	           bytesPerRow:kinc_g5_texture_stride(tex)
	         bytesPerImage:kinc_g5_texture_stride(tex) * tex->height];
	texture->_uploaded = false;
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level) {
	if (!texture->impl.has_mipmaps) {
		id<MTLDevice> device = getMetalDevice();
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert_image_format((kinc_image_format_t)texture->format)
		                                                                                      width:texture->width
		                                                                                     height:texture->height
		                                                                                  mipmapped:YES];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = texture->width;
		descriptor.height = texture->height;
		descriptor.depth = 1;
		descriptor.pixelFormat = convert_image_format((kinc_image_format_t)texture->format);
		descriptor.arrayLength = 1;
		bool writable = true;
		if (writable) {
			descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
		}
		void *mipmaptex = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];

		id<MTLCommandQueue> commandQueue = getMetalQueue();
		id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
		id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
		[commandEncoder copyFromTexture:(__bridge id<MTLTexture>)texture->impl._tex
		                    sourceSlice:0
		                    sourceLevel:0
		                   sourceOrigin:MTLOriginMake(0, 0, 0)
		                     sourceSize:MTLSizeMake(texture->width, texture->height, 1)
		                      toTexture:(__bridge id<MTLTexture>)mipmaptex
		               destinationSlice:0
		               destinationLevel:0
		              destinationOrigin:MTLOriginMake(0, 0, 0)];

		[commandEncoder endEncoding];
		[commandBuffer commit];
		[commandBuffer waitUntilCompleted];

		id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)texture->impl._tex;
		tex = nil;
		texture->impl._tex = mipmaptex;

		texture->impl.has_mipmaps = true;
	}

	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._tex;
	[tex replaceRegion:MTLRegionMake2D(0, 0, mipmap->width, mipmap->height)
	       mipmapLevel:level
	         withBytes:mipmap->data
	       bytesPerRow:mipmap->width * formatByteSize(mipmap->format)];
}

static MTLPixelFormat convert_format(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return MTLPixelFormatRGBA32Float;
	case KINC_IMAGE_FORMAT_RGBA64:
		return MTLPixelFormatRGBA16Float;
	case KINC_IMAGE_FORMAT_R32:
		return MTLPixelFormatR32Float;
	case KINC_IMAGE_FORMAT_R16:
		return MTLPixelFormatR16Float;
	case KINC_IMAGE_FORMAT_R8:
		return MTLPixelFormatR8Unorm;
	case KINC_IMAGE_FORMAT_RGBA32:
	default:
		return MTLPixelFormatBGRA8Unorm;
	}
}

static void render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits, int framebuffer_index) {
	memset(target, 0, sizeof(kinc_g5_render_target_t));

	target->width = width;
	target->height = height;

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

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, -1);
	target->width = target->width = width;
	target->height = target->height = height;
	target->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
}

static int framebuffer_count = 0;

void kinc_g5_render_target_init_framebuffer(kinc_g5_render_target_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, framebuffer_count);
	framebuffer_count += 1;
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

void kinc_g5_render_target_set_depth_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source) {
	target->impl._depthTex = source->impl._depthTex;
}
