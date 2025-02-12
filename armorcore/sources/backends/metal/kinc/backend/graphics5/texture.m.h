#include <kinc/graphics5/texture.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/texture.h>
#include <kinc/image.h>
#include <kinc/log.h>
#import <Metal/Metal.h>

id getMetalDevice(void);

static MTLPixelFormat convert_image_format(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA32:
		return MTLPixelFormatRGBA8Unorm;
	case KINC_IMAGE_FORMAT_GREY8:
		return MTLPixelFormatR8Unorm;
	case KINC_IMAGE_FORMAT_RGB24:
		return MTLPixelFormatRGBA8Unorm;
	case KINC_IMAGE_FORMAT_RGBA128:
		return MTLPixelFormatRGBA32Float;
	case KINC_IMAGE_FORMAT_RGBA64:
		return MTLPixelFormatRGBA16Float;
	case KINC_IMAGE_FORMAT_A32:
		return MTLPixelFormatR32Float;
	case KINC_IMAGE_FORMAT_BGRA32:
		return MTLPixelFormatBGRA8Unorm;
	case KINC_IMAGE_FORMAT_A16:
		return MTLPixelFormatR16Float;
	}
}

static int formatByteSize(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_RGB24:
		return 4;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
		return 4;
	default:
		assert(false);
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
	texture->texWidth = width;
	texture->texHeight = height;
	texture->format = format;
	texture->impl.data = malloc(width * height * (format == KINC_IMAGE_FORMAT_GREY8 ? 1 : 4));
	create(texture, width, height, format, true);
}

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, struct kinc_image *image) {
	texture->texWidth = image->width;
	texture->texHeight = image->height;
	texture->format = image->format;
	texture->impl.data = NULL;
	create(texture, image->width, image->height, image->format, true);
	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._tex;
	[tex replaceRegion:MTLRegionMake2D(0, 0, texture->texWidth, texture->texHeight)
	       mipmapLevel:0
	             slice:0
	         withBytes:image->data
	       bytesPerRow:kinc_g5_texture_stride(texture)
	     bytesPerImage:kinc_g5_texture_stride(texture) * texture->texHeight];
}

void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->texWidth = width;
	texture->texHeight = height;
	texture->format = format;
	texture->impl.data = malloc(width * height * (format == KINC_IMAGE_FORMAT_GREY8 ? 1 : 4));
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

id getMetalDevice(void);
id getMetalEncoder(void);

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	switch (texture->format) {
	case KINC_IMAGE_FORMAT_GREY8:
		return texture->texWidth;
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGB24:
		return texture->texWidth * 4;
	case KINC_IMAGE_FORMAT_RGBA64:
		return texture->texWidth * 8;
	case KINC_IMAGE_FORMAT_RGBA128:
		return texture->texWidth * 16;
	case KINC_IMAGE_FORMAT_A16:
		return texture->texWidth * 2;
	case KINC_IMAGE_FORMAT_A32:
		return texture->texWidth * 4;
	}
}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	return (uint8_t *)texture->impl.data;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *tex) {
	id<MTLTexture> texture = (__bridge id<MTLTexture>)tex->impl._tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, tex->texWidth, tex->texHeight)
	           mipmapLevel:0
	                 slice:0
	             withBytes:tex->impl.data
	           bytesPerRow:kinc_g5_texture_stride(tex)
	         bytesPerImage:kinc_g5_texture_stride(tex) * tex->texHeight];
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_image_t *mipmap, int level) {
	if (!texture->impl.has_mipmaps) {
		id<MTLDevice> device = getMetalDevice();
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert_image_format((kinc_image_format_t)texture->format)
		                                                                                      width:texture->texWidth
		                                                                                     height:texture->texHeight
		                                                                                  mipmapped:YES];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = texture->texWidth;
		descriptor.height = texture->texHeight;
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
		                     sourceSize:MTLSizeMake(texture->texWidth, texture->texHeight, 1)
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
