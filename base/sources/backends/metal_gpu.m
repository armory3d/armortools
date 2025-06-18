#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iron_math.h>
#include <iron_system.h>
#include <iron_gpu.h>
#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

id getMetalLayer(void);
id getMetalDevice(void);
id getMetalQueue(void);

extern int constant_buffer_index;
bool iron_gpu_transpose_mat = true;
static id<MTLCommandBuffer> command_buffer = nil;
static id<MTLRenderCommandEncoder> command_encoder = nil;
static id<MTLArgumentEncoder> argument_encoder = nil;
static id<MTLBuffer> argument_buffer = nil;
static int argument_buffer_step;
static id<CAMetalDrawable> drawable;
static id<MTLTexture> framebuffer_depth;
static int depth_bits;
static bool has_depth = false;
static int framebuffer_count = 0;
static iron_gpu_texture_t *render_targets[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

typedef struct inst {
	iron_matrix4x4_t m;
	int i;
} inst_t;

static iron_gpu_raytrace_acceleration_structure_t *accel;
static iron_gpu_raytrace_pipeline_t *pipeline;
static iron_gpu_texture_t *output = NULL;
static iron_gpu_buffer_t *constant_buf;
static id<MTLComputePipelineState> _raytracing_pipeline;
static NSMutableArray *_primitive_accels;
static id<MTLAccelerationStructure> _instance_accel;
static dispatch_semaphore_t _semaphore;
static iron_gpu_texture_t *_texpaint0;
static iron_gpu_texture_t *_texpaint1;
static iron_gpu_texture_t *_texpaint2;
static iron_gpu_texture_t *_texenv;
static iron_gpu_texture_t *_texsobol;
static iron_gpu_texture_t *_texscramble;
static iron_gpu_texture_t *_texrank;
static iron_gpu_buffer_t *vb[16];
static iron_gpu_buffer_t *vb_last[16];
static iron_gpu_buffer_t *ib[16];
static int vb_count = 0;
static int vb_count_last = 0;
static inst_t instances[1024];
static int instances_count = 0;

static MTLBlendFactor convert_blending_factor(iron_gpu_blending_factor_t factor) {
	switch (factor) {
	case IRON_GPU_BLEND_ONE:
		return MTLBlendFactorOne;
	case IRON_GPU_BLEND_ZERO:
		return MTLBlendFactorZero;
	case IRON_GPU_BLEND_SOURCE_ALPHA:
		return MTLBlendFactorSourceAlpha;
	case IRON_GPU_BLEND_DEST_ALPHA:
		return MTLBlendFactorDestinationAlpha;
	case IRON_GPU_BLEND_INV_SOURCE_ALPHA:
		return MTLBlendFactorOneMinusSourceAlpha;
	case IRON_GPU_BLEND_INV_DEST_ALPHA:
		return MTLBlendFactorOneMinusDestinationAlpha;
	}
}

static MTLBlendOperation convert_blending_operation(iron_gpu_blending_operation_t op) {
	switch (op) {
	case IRON_GPU_BLENDOP_ADD:
		return MTLBlendOperationAdd;
	}
}

static MTLCompareFunction convert_compare_mode(iron_gpu_compare_mode_t compare) {
	switch (compare) {
	case IRON_GPU_COMPARE_MODE_ALWAYS:
		return MTLCompareFunctionAlways;
	case IRON_GPU_COMPARE_MODE_NEVER:
		return MTLCompareFunctionNever;
	case IRON_GPU_COMPARE_MODE_LESS:
		return MTLCompareFunctionLess;
	}
}

static MTLCullMode convert_cull_mode(iron_gpu_cull_mode_t cull) {
	switch (cull) {
	case IRON_GPU_CULL_MODE_CLOCKWISE:
		return MTLCullModeFront;
	case IRON_GPU_CULL_MODE_COUNTERCLOCKWISE:
		return MTLCullModeBack;
	case IRON_GPU_CULL_MODE_NEVER:
		return MTLCullModeNone;
	}
}

static MTLPixelFormat convert_render_target_format(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return MTLPixelFormatRGBA32Float;
	case IRON_IMAGE_FORMAT_RGBA64:
		return MTLPixelFormatRGBA16Float;
	case IRON_IMAGE_FORMAT_R32:
		return MTLPixelFormatR32Float;
	case IRON_IMAGE_FORMAT_R16:
		return MTLPixelFormatR16Float;
	case IRON_IMAGE_FORMAT_R8:
		return MTLPixelFormatR8Unorm;
	case IRON_IMAGE_FORMAT_RGBA32:
	default:
		return MTLPixelFormatBGRA8Unorm;
	}
}

static MTLPixelFormat convert_image_format(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA32:
		return MTLPixelFormatRGBA8Unorm;
	case IRON_IMAGE_FORMAT_R8:
		return MTLPixelFormatR8Unorm;
	case IRON_IMAGE_FORMAT_R16:
		return MTLPixelFormatR16Float;
	case IRON_IMAGE_FORMAT_R32:
		return MTLPixelFormatR32Float;
	case IRON_IMAGE_FORMAT_RGBA128:
		return MTLPixelFormatRGBA32Float;
	case IRON_IMAGE_FORMAT_RGBA64:
		return MTLPixelFormatRGBA16Float;
	}
}

static int formatSize(MTLPixelFormat format) {
	switch (format) {
	case MTLPixelFormatRGBA32Float:
		return 16;
	case MTLPixelFormatRGBA16Float:
		return 8;
	case MTLPixelFormatR16Float:
		return 2;
	case MTLPixelFormatR8Unorm:
		return 1;
	default:
		return 4;
	}
}

static int formatByteSize(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return 16;
	case IRON_IMAGE_FORMAT_RGBA64:
		return 8;
	case IRON_IMAGE_FORMAT_R16:
		return 2;
	case IRON_IMAGE_FORMAT_R8:
		return 1;
	case IRON_IMAGE_FORMAT_RGBA32:
	case IRON_IMAGE_FORMAT_R32:
	default:
		return 4;
	}
}

void iron_gpu_destroy(void) {
}

void iron_gpu_internal_resize(int width, int height) {

}

id<MTLSamplerState> linear_sampler;

void iron_gpu_init(int depth_buffer_bits, bool vsync) {
	depth_bits = depth_buffer_bits;
	id<MTLDevice> device = getMetalDevice();

    MTLSamplerDescriptor *linear_desc = [MTLSamplerDescriptor new];
    linear_desc.minFilter = MTLSamplerMinMagFilterLinear;
    linear_desc.magFilter = MTLSamplerMinMagFilterLinear;
    linear_desc.mipFilter = MTLSamplerMipFilterLinear;
    linear_desc.sAddressMode = MTLSamplerAddressModeRepeat;
    linear_desc.tAddressMode = MTLSamplerAddressModeRepeat;
	linear_desc.supportArgumentBuffers = true;
	linear_sampler = [device newSamplerStateWithDescriptor:linear_desc];

	MTLArgumentDescriptor *constantsDesc = [MTLArgumentDescriptor argumentDescriptor];
    constantsDesc.dataType = MTLDataTypePointer;
    constantsDesc.index = 0;

	MTLArgumentDescriptor *samplerDesc = [MTLArgumentDescriptor argumentDescriptor];
    samplerDesc.dataType = MTLDataTypeSampler;
    samplerDesc.index = 1;

    MTLArgumentDescriptor *textureDesc0 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc0.dataType = MTLDataTypeTexture;
    textureDesc0.index = 2;
    textureDesc0.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc1 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc1.dataType = MTLDataTypeTexture;
    textureDesc1.index = 3;
    textureDesc1.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc2 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc2.dataType = MTLDataTypeTexture;
    textureDesc2.index = 4;
    textureDesc2.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc3 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc3.dataType = MTLDataTypeTexture;
    textureDesc3.index = 5;
    textureDesc3.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc4 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc4.dataType = MTLDataTypeTexture;
    textureDesc4.index = 6;
    textureDesc4.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc5 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc5.dataType = MTLDataTypeTexture;
    textureDesc5.index = 7;
    textureDesc5.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc6 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc6.dataType = MTLDataTypeTexture;
    textureDesc6.index = 8;
    textureDesc6.textureType = MTLTextureType2D;

	MTLArgumentDescriptor *textureDesc7 = [MTLArgumentDescriptor argumentDescriptor];
    textureDesc7.dataType = MTLDataTypeTexture;
    textureDesc7.index = 9;
    textureDesc7.textureType = MTLTextureType2D;

    NSArray *arguments = [NSArray arrayWithObjects:constantsDesc, samplerDesc, textureDesc0, textureDesc1, textureDesc2, textureDesc3, textureDesc4, textureDesc5, textureDesc6, textureDesc7, nil];
    argument_encoder = [device newArgumentEncoderWithArguments:arguments];

	// device.makeArgumentEncoder(bufferIndex:)

	argument_buffer_step = [argument_encoder encodedLength];
	// int align = [argument_encoder alignment];
	// argument_buffer_step += (align - (argument_buffer_step % align)) % align;
	argument_buffer = [device newBufferWithLength:(argument_buffer_step * 2048) options:MTLResourceStorageModeShared];
}

void iron_gpu_begin(iron_gpu_texture_t *target) {
	CAMetalLayer *layer = getMetalLayer();
	drawable = [layer nextDrawable];

	if (depth_bits > 0 && (framebuffer_depth == nil || framebuffer_depth.width != drawable.texture.width || framebuffer_depth.height != drawable.texture.height)) {
		if (framebuffer_depth != nil) {
			framebuffer_depth = nil;
		}
		MTLTextureDescriptor *desc = [MTLTextureDescriptor new];
		desc.textureType = MTLTextureType2D;
		desc.width = drawable.texture.width;
		desc.height = drawable.texture.height;
		desc.depth = 1;
		desc.pixelFormat = MTLPixelFormatDepth32Float;
		desc.arrayLength = 1;
		desc.mipmapLevelCount = 1;
		desc.resourceOptions = MTLResourceStorageModePrivate;
		desc.usage = MTLTextureUsageRenderTarget;
		id<MTLDevice> device = getMetalDevice();
		framebuffer_depth = [device newTextureWithDescriptor:desc];
	}
	has_depth = (depth_bits > 0 && framebuffer_depth != nil);
}

void iron_gpu_end() {
	[command_encoder endEncoding];
	[command_buffer presentDrawable:drawable];
	[command_buffer commit];
	[command_buffer waitUntilCompleted];

	drawable = nil;
	command_buffer = nil;
	command_encoder = nil;
}

int iron_gpu_max_bound_textures(void) {
	return 16;
}

void iron_gpu_command_list_init(iron_gpu_command_list_t *list) {
}

void iron_gpu_command_list_destroy(iron_gpu_command_list_t *list) {
}

void iron_gpu_command_list_begin(iron_gpu_command_list_t *list) {
	render_targets[0] = NULL;
}

void iron_gpu_command_list_end(iron_gpu_command_list_t *list) {
}

void iron_gpu_command_list_draw(iron_gpu_command_list_t *list) {
	id<MTLBuffer> indexBuffer = (__bridge id<MTLBuffer>)list->impl.current_index_buffer->impl.metal_buffer;
	[command_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
	                    		indexCount:iron_gpu_index_buffer_count(list->impl.current_index_buffer)
	                     		 indexType:MTLIndexTypeUInt32
	                   		   indexBuffer:indexBuffer
	             		 indexBufferOffset:0];
}

void iron_gpu_command_list_viewport(iron_gpu_command_list_t *list, int x, int y, int width, int height) {
	MTLViewport viewport;
	viewport.originX = x;
	viewport.originY = y;
	viewport.width = width;
	viewport.height = height;
	viewport.znear = 0.1;
	viewport.zfar = 100.0;
	[command_encoder setViewport:viewport];
}

void iron_gpu_command_list_scissor(iron_gpu_command_list_t *list, int x, int y, int width, int height) {
	MTLScissorRect scissor;
	scissor.x = x;
	scissor.y = y;
	int target_w = -1;
	int target_h = -1;
	if (render_targets[0] != NULL) {
		target_w = render_targets[0]->width;
		target_h = render_targets[0]->height;
	}
	else {
		target_w = iron_window_width();
		target_h = iron_window_height();
	}
	scissor.width = (x + width <= target_w) ? width : target_w - x;
	scissor.height = (y + height <= target_h) ? height : target_h - y;
	[command_encoder setScissorRect:scissor];
}

void iron_gpu_command_list_disable_scissor(iron_gpu_command_list_t *list) {
	MTLScissorRect scissor;
	scissor.x = 0;
	scissor.y = 0;
	if (render_targets[0] != NULL) {
		scissor.width = render_targets[0]->width;
		scissor.height = render_targets[0]->height;
	}
	else {
		scissor.width = iron_window_width();
		scissor.height = iron_window_height();
	}
	[command_encoder setScissorRect:scissor];
}

void iron_gpu_command_list_set_pipeline(iron_gpu_command_list_t *list, iron_gpu_pipeline_t *pipeline) {
	if (has_depth) {
		id<MTLRenderPipelineState> pipe = (__bridge id<MTLRenderPipelineState>)pipeline->impl._pipelineDepth;
		[command_encoder setRenderPipelineState:pipe];
		id<MTLDepthStencilState> depthStencil = (__bridge id<MTLDepthStencilState>)pipeline->impl._depthStencil;
		[command_encoder setDepthStencilState:depthStencil];
	}
	else {
		id<MTLRenderPipelineState> pipe = (__bridge id<MTLRenderPipelineState>)pipeline->impl._pipeline;
		[command_encoder setRenderPipelineState:pipe];
		id<MTLDepthStencilState> depthStencil = (__bridge id<MTLDepthStencilState>)pipeline->impl._depthStencilNone;
		[command_encoder setDepthStencilState:depthStencil];
	}
	[command_encoder setFrontFacingWinding:MTLWindingClockwise];
	[command_encoder setCullMode:convert_cull_mode(pipeline->cull_mode)];
}

void iron_gpu_command_list_set_vertex_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buf) {
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.metal_buffer;
	[command_encoder setVertexBuffer:buffer offset:0 atIndex:0];
}

void iron_gpu_command_list_set_index_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer) {
	list->impl.current_index_buffer = buffer;
}

void iron_gpu_command_list_set_render_targets(iron_gpu_command_list_t *list, iron_gpu_texture_t **targets, int count, unsigned flags, unsigned color, float depth) {
	if (command_buffer != nil && command_encoder != nil) {
		[command_encoder endEncoding];
		[command_buffer commit];
		[command_buffer waitUntilCompleted];
	}

	if (targets[0]->framebuffer_index >= 0) {
		for (int i = 0; i < 8; ++i) {
			render_targets[i] = NULL;
		}
		targets = NULL;
		count = 1;
	}
	else {
		for (int i = 0; i < count; ++i) {
			render_targets[i] = targets[i];
		}
	}

	MTLRenderPassDescriptor *desc = [MTLRenderPassDescriptor renderPassDescriptor];
	for (int i = 0; i < count; ++i) {
		if (targets == NULL) {
			desc.colorAttachments[i].texture = drawable.texture;
			desc.depthAttachment.texture = framebuffer_depth;
			has_depth = framebuffer_depth != nil;
		}
		else {
			desc.colorAttachments[i].texture = (__bridge id<MTLTexture>)targets[i]->impl._tex;
			desc.depthAttachment.texture = (__bridge id<MTLTexture>)targets[0]->impl._depthTex;
			has_depth = targets[0]->impl._depthTex != nil;
		}
		if (flags & IRON_GPU_CLEAR_COLOR) {
			float red, green, blue, alpha;
			iron_color_components(color, &red, &green, &blue, &alpha);
			desc.colorAttachments[i].loadAction = MTLLoadActionClear;
			desc.colorAttachments[i].storeAction = MTLStoreActionStore;
			desc.colorAttachments[i].clearColor = MTLClearColorMake(red, green, blue, alpha);
		}
		else {
			desc.colorAttachments[i].loadAction = MTLLoadActionLoad;
			desc.colorAttachments[i].storeAction = MTLStoreActionStore;
			desc.colorAttachments[i].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
		}
	}

	if (flags & IRON_GPU_CLEAR_DEPTH) {
		desc.depthAttachment.clearDepth = depth;
		desc.depthAttachment.loadAction = MTLLoadActionClear;
		desc.depthAttachment.storeAction = MTLStoreActionStore;
	}
	else {
		desc.depthAttachment.clearDepth = 1;
		desc.depthAttachment.loadAction = MTLLoadActionLoad;
		desc.depthAttachment.storeAction = MTLStoreActionStore;
	}

	id<MTLCommandQueue> commandQueue = getMetalQueue();
	command_buffer = [commandQueue commandBuffer];
	command_encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
}

void iron_gpu_command_list_upload_index_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer) {
}

void iron_gpu_command_list_upload_vertex_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer) {
}

void iron_gpu_command_list_upload_texture(iron_gpu_command_list_t *list, iron_gpu_texture_t *texture) {
}

void iron_gpu_command_list_get_render_target_pixels(iron_gpu_command_list_t *list, iron_gpu_texture_t *render_target, uint8_t *data) {
	// Create readback buffer
	if (render_target->impl._texReadback == NULL) {
		id<MTLDevice> device = getMetalDevice();
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = render_target->width;
		descriptor.height = render_target->height;
		descriptor.depth = 1;
		descriptor.pixelFormat = [(__bridge id<MTLTexture>)render_target->impl._tex pixelFormat];
		descriptor.arrayLength = 1;
		descriptor.mipmapLevelCount = 1;
		descriptor.usage = MTLTextureUsageUnknown;
		descriptor.resourceOptions = MTLResourceStorageModeShared;
		render_target->impl._texReadback = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];
	}

	// Copy render target to readback buffer
	id<MTLCommandQueue> commandQueue = getMetalQueue();
	id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
	id<MTLBlitCommandEncoder> commandEncoder = [commandBuffer blitCommandEncoder];
	[commandEncoder copyFromTexture:(__bridge id<MTLTexture>)render_target->impl._tex
	                    sourceSlice:0
	                    sourceLevel:0
	                   sourceOrigin:MTLOriginMake(0, 0, 0)
	                     sourceSize:MTLSizeMake(render_target->width, render_target->height, 1)
	                      toTexture:(__bridge id<MTLTexture>)render_target->impl._texReadback
	               destinationSlice:0
	               destinationLevel:0
	              destinationOrigin:MTLOriginMake(0, 0, 0)];
	[commandEncoder endEncoding];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];

	// Read buffer
	id<MTLTexture> tex = (__bridge id<MTLTexture>)render_target->impl._texReadback;
	int formatByteSize = formatSize([(__bridge id<MTLTexture>)render_target->impl._tex pixelFormat]);
	MTLRegion region = MTLRegionMake2D(0, 0, render_target->width, render_target->height);
	[tex getBytes:data bytesPerRow:formatByteSize * render_target->width fromRegion:region mipmapLevel:0];
}

void iron_gpu_command_list_wait(iron_gpu_command_list_t *list) {

}

void iron_gpu_command_list_set_constant_buffer(iron_gpu_command_list_t *list, iron_gpu_buffer_t *buffer, int offset, size_t size) {
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	int i = constant_buffer_index;
	[argument_encoder setArgumentBuffer:argument_buffer offset:argument_buffer_step * i];
	[argument_encoder setBuffer:buf offset:offset atIndex:0];
	[argument_encoder setSamplerState:linear_sampler atIndex:1];
	[command_encoder setVertexBuffer:argument_buffer offset:argument_buffer_step * i atIndex:1];
    [command_encoder setFragmentBuffer:argument_buffer offset:argument_buffer_step * i atIndex:1];
	[command_encoder useResource:buf usage:MTLResourceUsageRead  stages:MTLRenderStageVertex|MTLRenderStageFragment];
}

void iron_gpu_command_list_set_texture(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t *unit, iron_gpu_texture_t *texture) {
	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._tex;
	int i = constant_buffer_index;
	[argument_encoder setArgumentBuffer:argument_buffer offset:argument_buffer_step * i];
	[argument_encoder setTexture:tex atIndex:unit->offset + 2];
	[command_encoder useResource:tex usage:MTLResourceUsageRead stages:MTLRenderStageVertex|MTLRenderStageFragment];
}

void iron_gpu_set_texture_depth(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t *unit, iron_gpu_texture_t *target) {
	id<MTLTexture> depth_tex = (__bridge id<MTLTexture>)target->impl._depthTex;
	int i = constant_buffer_index;
	[argument_encoder setArgumentBuffer:argument_buffer offset:argument_buffer_step * i];
	[argument_encoder setTexture:depth_tex atIndex:unit->offset + 2];
	[command_encoder useResource:depth_tex usage:MTLResourceUsageRead stages:MTLRenderStageVertex|MTLRenderStageFragment];
}

void iron_gpu_pipeline_init(iron_gpu_pipeline_t *pipeline) {
	memset(&pipeline->impl, 0, sizeof(pipeline->impl));
	gpu_internal_pipeline_init(pipeline);
}

void iron_gpu_pipeline_destroy(iron_gpu_pipeline_t *pipeline) {
	id<MTLRenderPipelineState> pipe = (__bridge_transfer id<MTLRenderPipelineState>)pipeline->impl._pipeline;
	pipe = nil;
	pipeline->impl._pipeline = NULL;

	id<MTLRenderPipelineState> pipeDepth = (__bridge_transfer id<MTLRenderPipelineState>)pipeline->impl._pipelineDepth;
	pipeDepth = nil;
	pipeline->impl._pipelineDepth = NULL;

	id<MTLDepthStencilState> depthStencil = (__bridge_transfer id<MTLDepthStencilState>)pipeline->impl._depthStencil;
	depthStencil = nil;
	pipeline->impl._depthStencil = NULL;

	id<MTLDepthStencilState> depthStencilNone = (__bridge_transfer id<MTLDepthStencilState>)pipeline->impl._depthStencilNone;
	depthStencilNone = nil;
	pipeline->impl._depthStencilNone = NULL;
}

void iron_gpu_pipeline_compile(iron_gpu_pipeline_t *pipeline) {
	id<MTLDevice> device = getMetalDevice();
	NSError *error = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:pipeline->vertex_shader->impl.source length:pipeline->vertex_shader->impl.length encoding:NSUTF8StringEncoding] options:nil error:&error];
	if (library == nil) {
		iron_error("%s", error.localizedDescription.UTF8String);
	}

	pipeline->vertex_shader->impl.mtlFunction = (__bridge_retained void *)[library newFunctionWithName:[NSString stringWithCString:pipeline->vertex_shader->impl.name encoding:NSUTF8StringEncoding]];
	assert(pipeline->vertex_shader->impl.mtlFunction);

	pipeline->fragment_shader->impl.mtlFunction = (__bridge_retained void *)[library newFunctionWithName:[NSString stringWithCString:pipeline->fragment_shader->impl.name encoding:NSUTF8StringEncoding]];
	assert(pipeline->fragment_shader->impl.mtlFunction);

	MTLRenderPipelineDescriptor *renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = (__bridge id<MTLFunction>)pipeline->vertex_shader->impl.mtlFunction;
	renderPipelineDesc.fragmentFunction = (__bridge id<MTLFunction>)pipeline->fragment_shader->impl.mtlFunction;

	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		renderPipelineDesc.colorAttachments[i].pixelFormat = convert_render_target_format(pipeline->color_attachment[i]);
		renderPipelineDesc.colorAttachments[i].blendingEnabled =
		    pipeline->blend_source != IRON_GPU_BLEND_ONE || pipeline->blend_destination != IRON_GPU_BLEND_ZERO ||
		    pipeline->alpha_blend_source != IRON_GPU_BLEND_ONE || pipeline->alpha_blend_destination != IRON_GPU_BLEND_ZERO;
		renderPipelineDesc.colorAttachments[i].sourceRGBBlendFactor = convert_blending_factor(pipeline->blend_source);
		renderPipelineDesc.colorAttachments[i].destinationRGBBlendFactor = convert_blending_factor(pipeline->blend_destination);
		renderPipelineDesc.colorAttachments[i].rgbBlendOperation = convert_blending_operation(pipeline->blend_operation);
		renderPipelineDesc.colorAttachments[i].sourceAlphaBlendFactor = convert_blending_factor(pipeline->alpha_blend_source);
		renderPipelineDesc.colorAttachments[i].destinationAlphaBlendFactor = convert_blending_factor(pipeline->alpha_blend_destination);
		renderPipelineDesc.colorAttachments[i].alphaBlendOperation = convert_blending_operation(pipeline->alpha_blend_operation);
		renderPipelineDesc.colorAttachments[i].writeMask =
		    (pipeline->color_write_mask_red[i] ? MTLColorWriteMaskRed : 0) |
			(pipeline->color_write_mask_green[i] ? MTLColorWriteMaskGreen : 0) |
		    (pipeline->color_write_mask_blue[i] ? MTLColorWriteMaskBlue : 0) |
			(pipeline->color_write_mask_alpha[i] ? MTLColorWriteMaskAlpha : 0);
	}
	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatInvalid;

	float offset = 0;
	MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];

	for (int i = 0; i < pipeline->input_layout->size; ++i) {
		vertexDescriptor.attributes[i].bufferIndex = 0;
		vertexDescriptor.attributes[i].offset = offset;
		offset += iron_gpu_vertex_data_size(pipeline->input_layout->elements[i].data);

		switch (pipeline->input_layout->elements[i].data) {
		case IRON_GPU_VERTEX_DATA_F32_1X:
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat;
			break;
		case IRON_GPU_VERTEX_DATA_F32_2X:
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat2;
			break;
		case IRON_GPU_VERTEX_DATA_F32_3X:
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat3;
			break;
		case IRON_GPU_VERTEX_DATA_F32_4X:
			vertexDescriptor.attributes[i].format = MTLVertexFormatFloat4;
			break;
		case IRON_GPU_VERTEX_DATA_I16_2X_NORM:
			vertexDescriptor.attributes[i].format = MTLVertexFormatShort2Normalized;
			break;
		case IRON_GPU_VERTEX_DATA_I16_4X_NORM:
			vertexDescriptor.attributes[i].format = MTLVertexFormatShort4Normalized;
			break;
		}
	}

	vertexDescriptor.layouts[0].stride = offset;
	vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	renderPipelineDesc.vertexDescriptor = vertexDescriptor;

	NSError *errors = nil;
	MTLRenderPipelineReflection *reflection = nil;

	pipeline->impl._pipeline = (__bridge_retained void *)[
		device newRenderPipelineStateWithDescriptor:renderPipelineDesc
	                                        options:MTLPipelineOptionBufferTypeInfo
	                                     reflection:&reflection
	                                          error:&errors];

	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
	pipeline->impl._pipelineDepth = (__bridge_retained void *)[
		device newRenderPipelineStateWithDescriptor:renderPipelineDesc
	                                        options:MTLPipelineOptionBufferTypeInfo
	                                     reflection:&reflection
	                                          error:&errors];

	MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
	depthStencilDescriptor.depthCompareFunction = convert_compare_mode(pipeline->depth_mode);
	depthStencilDescriptor.depthWriteEnabled = pipeline->depth_write;
	pipeline->impl._depthStencil = (__bridge_retained void *)[device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

	depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
	depthStencilDescriptor.depthWriteEnabled = false;
	pipeline->impl._depthStencilNone = (__bridge_retained void *)[device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

iron_gpu_constant_location_t iron_gpu_pipeline_get_constant_location(iron_gpu_pipeline_t *pipeline, const char *name) {
	iron_gpu_constant_location_t location;
	return location;
}

iron_gpu_texture_unit_t iron_gpu_pipeline_get_texture_unit(iron_gpu_pipeline_t *pipeline, const char *name) {
	iron_gpu_texture_unit_t unit;
	unit.offset = -1;
	return unit;
}

void iron_gpu_shader_destroy(iron_gpu_shader_t *shader) {
	id<MTLFunction> function = (__bridge_transfer id<MTLFunction>)shader->impl.mtlFunction;
	function = nil;
	shader->impl.mtlFunction = NULL;
}

void iron_gpu_shader_init(iron_gpu_shader_t *shader, const void *data, size_t length, iron_gpu_shader_type_t type) {
	shader->impl.name[0] = 0;
	const char *source = data;

	for (int i = 3; i < length; ++i) { // //>
		if (source[i] == '\n') {
			shader->impl.name[i - 3] = 0;
			break;
		}
		shader->impl.name[i - 3] = source[i];
	}

	shader->impl.source = data;
	shader->impl.length = length;
}

void iron_gpu_raytrace_pipeline_init(iron_gpu_raytrace_pipeline_t *pipeline, iron_gpu_command_list_t *command_list, void *ray_shader, int ray_shader_size, iron_gpu_buffer_t *constant_buffer) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
	constant_buf = constant_buffer;

	NSError *error = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:ray_shader length:ray_shader_size encoding:NSUTF8StringEncoding]
	                                              options:nil
	                                                error:&error];
	if (library == nil) {
		iron_error("%s", error.localizedDescription.UTF8String);
	}

	MTLComputePipelineDescriptor *descriptor = [[MTLComputePipelineDescriptor alloc] init];
	descriptor.computeFunction = [library newFunctionWithName:@"raytracingKernel"];
	descriptor.threadGroupSizeIsMultipleOfThreadExecutionWidth = YES;
	_raytracing_pipeline = [device newComputePipelineStateWithDescriptor:descriptor options:0 reflection:nil error:&error];
	_semaphore = dispatch_semaphore_create(2);
}

void iron_gpu_raytrace_pipeline_destroy(iron_gpu_raytrace_pipeline_t *pipeline) {
}

static void create(iron_gpu_texture_t *texture, int width, int height, int format, bool writable) {
	texture->impl.has_mipmaps = false;
	id<MTLDevice> device = getMetalDevice();

	MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert_image_format((iron_image_format_t)format)
	                                                                                      width:width
	                                                                                     height:height
	                                                                                  mipmapped:NO];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = convert_image_format((iron_image_format_t)format);
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	// TODO: Make less textures writable
	if (writable) {
		descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
	}

	texture->impl._tex = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];
}

void iron_gpu_texture_init(iron_gpu_texture_t *texture, int width, int height, iron_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->impl.data = malloc(width * height * (format == IRON_IMAGE_FORMAT_R8 ? 1 : 4));
	create(texture, width, height, format, true);
	texture->uploaded = true;
	texture->data = NULL;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	texture->framebuffer_index = -1;
}

void iron_gpu_texture_init_from_bytes(iron_gpu_texture_t *texture, void *data, int width, int height, iron_image_format_t format) {
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->data = data;
	texture->uploaded = false;
	texture->impl.data = NULL;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	texture->framebuffer_index = -1;
	create(texture, width, height, format, true);
	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._tex;
	[tex replaceRegion:MTLRegionMake2D(0, 0, texture->width, texture->height)
	       mipmapLevel:0
	             slice:0
	         withBytes:data
	       bytesPerRow:iron_gpu_texture_stride(texture)
	     bytesPerImage:iron_gpu_texture_stride(texture) * texture->height];
}

void iron_gpu_texture_destroy(iron_gpu_texture_t *target) {
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

	if (target->impl.data != NULL) {
		free(target->impl.data);
		target->impl.data = NULL;
	}
}

int iron_gpu_texture_stride(iron_gpu_texture_t *texture) {
	switch (texture->format) {
	case IRON_IMAGE_FORMAT_R8:
		return texture->width;
	case IRON_IMAGE_FORMAT_RGBA32:
	default:
		return texture->width * 4;
	case IRON_IMAGE_FORMAT_RGBA64:
		return texture->width * 8;
	case IRON_IMAGE_FORMAT_RGBA128:
		return texture->width * 16;
	}
}

void iron_gpu_texture_generate_mipmaps(iron_gpu_texture_t *texture, int levels) {}

void iron_gpu_texture_set_mipmap(iron_gpu_texture_t *texture, iron_gpu_texture_t *mipmap, int level) {
	if (!texture->impl.has_mipmaps) {
		id<MTLDevice> device = getMetalDevice();
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert_image_format((iron_image_format_t)texture->format)
		                                                                                      width:texture->width
		                                                                                     height:texture->height
		                                                                                  mipmapped:YES];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = texture->width;
		descriptor.height = texture->height;
		descriptor.depth = 1;
		descriptor.pixelFormat = convert_image_format((iron_image_format_t)texture->format);
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

static void render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depth_buffer_bits, int framebuffer_index) {
	memset(target, 0, sizeof(iron_gpu_texture_t));
	target->width = width;
	target->height = height;
	target->data = NULL;
	target->uploaded = true;
	target->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
	target->framebuffer_index = framebuffer_index;

	id<MTLDevice> device = getMetalDevice();
	MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = convert_render_target_format(format);
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
	descriptor.resourceOptions = MTLResourceStorageModePrivate;
	target->impl._tex = (__bridge_retained void *)[device newTextureWithDescriptor:descriptor];

	if (depth_buffer_bits > 0) {
		MTLTextureDescriptor *depthDescriptor = [MTLTextureDescriptor new];
		depthDescriptor.textureType = MTLTextureType2D;
		depthDescriptor.width = width;
		depthDescriptor.height = height;
		depthDescriptor.depth = 1;
		depthDescriptor.pixelFormat = MTLPixelFormatDepth32Float;
		depthDescriptor.arrayLength = 1;
		depthDescriptor.mipmapLevelCount = 1;
		depthDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
		depthDescriptor.resourceOptions = MTLResourceStorageModePrivate;
		target->impl._depthTex = (__bridge_retained void *)[device newTextureWithDescriptor:depthDescriptor];
	}

	target->impl._texReadback = NULL;
}

void iron_gpu_render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depth_buffer_bits) {
	render_target_init(target, width, height, format, depth_buffer_bits, -1);
	target->width = target->width = width;
	target->height = target->height = height;
	target->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
	target->uploaded = true;
}

void iron_gpu_render_target_init_framebuffer(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depth_buffer_bits) {
	render_target_init(target, width, height, format, depth_buffer_bits, framebuffer_count);
	framebuffer_count += 1;
}

void iron_gpu_render_target_set_depth_from(iron_gpu_texture_t *target, iron_gpu_texture_t *source) {
	target->impl._depthTex = source->impl._depthTex;
}

void iron_gpu_vertex_buffer_init(iron_gpu_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, bool gpu_memory) {
	memset(&buffer->impl, 0, sizeof(buffer->impl));
	buffer->count = count;
	buffer->impl.gpu_memory = gpu_memory;
	for (int i = 0; i < structure->size; ++i) {
		iron_gpu_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += iron_gpu_vertex_data_size(element.data);
	}

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
	options |= MTLResourceStorageModeShared;

	id<MTLBuffer> buf = [device newBufferWithLength:count * buffer->impl.myStride options:options];
	buffer->impl.metal_buffer = (__bridge_retained void *)buf;
}

void iron_gpu_vertex_buffer_destroy(iron_gpu_buffer_t *buf) {
	id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)buf->impl.metal_buffer;
	buffer = nil;
	buf->impl.metal_buffer = NULL;
}

float *iron_gpu_vertex_buffer_lock(iron_gpu_buffer_t *buf) {
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.metal_buffer;
	float *floats = (float *)[buffer contents];
	return floats;
}

void iron_gpu_vertex_buffer_unlock(iron_gpu_buffer_t *buf) {
}

int iron_gpu_vertex_buffer_count(iron_gpu_buffer_t *buffer) {
	return buffer->count;
}

int iron_gpu_vertex_buffer_stride(iron_gpu_buffer_t *buffer) {
	return buffer->impl.myStride;
}

void iron_gpu_constant_buffer_init(iron_gpu_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;
	buffer->impl._buffer = (__bridge_retained void *)[getMetalDevice() newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
}

void iron_gpu_constant_buffer_destroy(iron_gpu_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl._buffer;
	buf = nil;
	buffer->impl._buffer = NULL;
}

void iron_gpu_constant_buffer_lock(iron_gpu_buffer_t *buffer, int start, int count) {
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	uint8_t *data = (uint8_t *)[buf contents];
	buffer->data = &data[start];
}

void iron_gpu_constant_buffer_unlock(iron_gpu_buffer_t *buffer) {
	// buffer->data = NULL;
}

int iron_gpu_constant_buffer_size(iron_gpu_buffer_t *buffer) {
	return buffer->impl.mySize;
}

void iron_gpu_index_buffer_init(iron_gpu_buffer_t *buffer, int indexCount, bool gpu_memory) {
	buffer->impl.count = indexCount;
	buffer->impl.gpu_memory = gpu_memory;

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
	options |= MTLResourceStorageModeShared;

	buffer->impl.metal_buffer = (__bridge_retained void *)[device
	    newBufferWithLength:sizeof(uint32_t) * indexCount
	                options:options];
}

void iron_gpu_index_buffer_destroy(iron_gpu_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl.metal_buffer;
	buf = nil;
	buffer->impl.metal_buffer = NULL;
}

static int iron_gpu_internal_index_buffer_stride(iron_gpu_buffer_t *buffer) {
	return 4;
}

void *iron_gpu_index_buffer_lock(iron_gpu_buffer_t *buffer) {
	int start = 0;
	int count = iron_gpu_index_buffer_count(buffer);
	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->impl.metal_buffer;
	uint8_t *data = (uint8_t *)[metal_buffer contents];
	return &data[start * iron_gpu_internal_index_buffer_stride(buffer)];
}

void iron_gpu_index_buffer_unlock(iron_gpu_buffer_t *buffer) {
}

int iron_gpu_index_buffer_count(iron_gpu_buffer_t *buffer) {
	return buffer->impl.count;
}

bool iron_gpu_raytrace_supported() {
	id<MTLDevice> device = getMetalDevice();
	return device.supportsRaytracing;
}

id<MTLAccelerationStructure> create_acceleration_sctructure(MTLAccelerationStructureDescriptor *descriptor) {
	id<MTLDevice> device = getMetalDevice();
	id<MTLCommandQueue> queue = getMetalQueue();

	MTLAccelerationStructureSizes accel_sizes = [device accelerationStructureSizesWithDescriptor:descriptor];
	id<MTLAccelerationStructure> acceleration_structure = [device newAccelerationStructureWithSize:accel_sizes.accelerationStructureSize];

	id<MTLBuffer> scratch_buffer = [device newBufferWithLength:accel_sizes.buildScratchBufferSize options:MTLResourceStorageModePrivate];
	id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
	id<MTLAccelerationStructureCommandEncoder> command_encoder = [command_buffer accelerationStructureCommandEncoder];
	id<MTLBuffer> compacteds_size_buffer = [device newBufferWithLength:sizeof(uint32_t) options:MTLResourceStorageModeShared];

	[command_encoder buildAccelerationStructure:acceleration_structure descriptor:descriptor scratchBuffer:scratch_buffer scratchBufferOffset:0];

	[command_encoder writeCompactedAccelerationStructureSize:acceleration_structure toBuffer:compacteds_size_buffer offset:0];

	[command_encoder endEncoding];
	[command_buffer commit];
	[command_buffer waitUntilCompleted];

	uint32_t compacted_size = *(uint32_t *)compacteds_size_buffer.contents;
	id<MTLAccelerationStructure> compacted_acceleration_structure = [device newAccelerationStructureWithSize:compacted_size];
	command_buffer = [queue commandBuffer];
	command_encoder = [command_buffer accelerationStructureCommandEncoder];
	[command_encoder copyAndCompactAccelerationStructure:acceleration_structure toAccelerationStructure:compacted_acceleration_structure];
	[command_encoder endEncoding];
	[command_buffer commit];

	return compacted_acceleration_structure;
}

void iron_gpu_raytrace_acceleration_structure_init(iron_gpu_raytrace_acceleration_structure_t *accel) {
	vb_count = 0;
	instances_count = 0;
}

void iron_gpu_raytrace_acceleration_structure_add(iron_gpu_raytrace_acceleration_structure_t *accel, iron_gpu_buffer_t *_vb, iron_gpu_buffer_t *_ib,
	iron_matrix4x4_t _transform) {

	int vb_i = -1;
	for (int i = 0; i < vb_count; ++i) {
		if (_vb == vb[i]) {
			vb_i = i;
			break;
		}
	}
	if (vb_i == -1) {
		vb_i = vb_count;
		vb[vb_count] = _vb;
		ib[vb_count] = _ib;
		vb_count++;
	}

	inst_t inst = { .i = vb_i, .m =  _transform };
	instances[instances_count] = inst;
	instances_count++;
}

void _iron_gpu_raytrace_acceleration_structure_destroy_bottom(iron_gpu_raytrace_acceleration_structure_t *accel) {
//	for (int i = 0; i < vb_count_last; ++i) {
//	}
	_primitive_accels = nil;
}

void _iron_gpu_raytrace_acceleration_structure_destroy_top(iron_gpu_raytrace_acceleration_structure_t *accel) {
	_instance_accel = nil;
}

void iron_gpu_raytrace_acceleration_structure_build(iron_gpu_raytrace_acceleration_structure_t *accel, iron_gpu_command_list_t *command_list,
	iron_gpu_buffer_t *_vb_full, iron_gpu_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_iron_gpu_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_iron_gpu_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) {
		return;
	}

	MTLResourceOptions options = MTLResourceStorageModeShared;

	MTLAccelerationStructureTriangleGeometryDescriptor *descriptor = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
	descriptor.indexType = MTLIndexTypeUInt32;
	descriptor.indexBuffer = (__bridge id<MTLBuffer>)ib[0]->impl.metal_buffer;
	descriptor.vertexBuffer = (__bridge id<MTLBuffer>)vb[0]->impl.metal_buffer;
	descriptor.vertexStride = vb[0]->impl.myStride;
	descriptor.triangleCount = ib[0]->impl.count / 3;
	descriptor.vertexFormat = MTLAttributeFormatShort4Normalized;

	MTLPrimitiveAccelerationStructureDescriptor *accel_descriptor = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
	accel_descriptor.geometryDescriptors = @[ descriptor ];
	id<MTLAccelerationStructure> acceleration_structure = create_acceleration_sctructure(accel_descriptor);
	_primitive_accels = [[NSMutableArray alloc] init];
	[_primitive_accels addObject:acceleration_structure];

	id<MTLBuffer> instance_buffer = [device newBufferWithLength:sizeof(MTLAccelerationStructureInstanceDescriptor) * 1 options:options];

	MTLAccelerationStructureInstanceDescriptor *instance_descriptors = (MTLAccelerationStructureInstanceDescriptor *)instance_buffer.contents;
	instance_descriptors[0].accelerationStructureIndex = 0;
	instance_descriptors[0].options = MTLAccelerationStructureInstanceOptionOpaque;
	instance_descriptors[0].mask = 1;
	instance_descriptors[0].transformationMatrix.columns[0] = MTLPackedFloat3Make(instances[0].m.m[0], instances[0].m.m[1], instances[0].m.m[2]);
	instance_descriptors[0].transformationMatrix.columns[1] = MTLPackedFloat3Make(instances[0].m.m[4], instances[0].m.m[5], instances[0].m.m[6]);
	instance_descriptors[0].transformationMatrix.columns[2] = MTLPackedFloat3Make(instances[0].m.m[8], instances[0].m.m[9], instances[0].m.m[10]);
	instance_descriptors[0].transformationMatrix.columns[3] = MTLPackedFloat3Make(instances[0].m.m[12], instances[0].m.m[13], instances[0].m.m[14]);

	MTLInstanceAccelerationStructureDescriptor *inst_accel_descriptor = [MTLInstanceAccelerationStructureDescriptor descriptor];
	inst_accel_descriptor.instancedAccelerationStructures = _primitive_accels;
	inst_accel_descriptor.instanceCount = 1;
	inst_accel_descriptor.instanceDescriptorBuffer = instance_buffer;
	_instance_accel = create_acceleration_sctructure(inst_accel_descriptor);
}

void iron_gpu_raytrace_acceleration_structure_destroy(iron_gpu_raytrace_acceleration_structure_t *accel) {}

void iron_gpu_raytrace_set_textures(iron_gpu_texture_t *texpaint0, iron_gpu_texture_t *texpaint1, iron_gpu_texture_t *texpaint2, iron_gpu_texture_t *texenv, iron_gpu_texture_t *texsobol, iron_gpu_texture_t *texscramble, iron_gpu_texture_t *texrank) {
	_texpaint0 = texpaint0;
	_texpaint1 = texpaint1;
	_texpaint2 = texpaint2;
	_texenv = texenv;
	_texsobol = texsobol;
	_texscramble = texscramble;
	_texrank = texrank;
}

void iron_gpu_raytrace_set_acceleration_structure(iron_gpu_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void iron_gpu_raytrace_set_pipeline(iron_gpu_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void iron_gpu_raytrace_set_target(iron_gpu_texture_t *_output) {
	output = _output;
}

void iron_gpu_raytrace_dispatch_rays(iron_gpu_command_list_t *command_list) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
	dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);

	id<MTLCommandQueue> queue = getMetalQueue();
	id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
	__block dispatch_semaphore_t sem = _semaphore;
	[command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		dispatch_semaphore_signal(sem);
	}];

	NSUInteger width = output->width;
	NSUInteger height = output->height;
	MTLSize threads_per_threadgroup = MTLSizeMake(8, 8, 1);
	MTLSize threadgroups = MTLSizeMake((width + threads_per_threadgroup.width - 1) / threads_per_threadgroup.width,
	                                   (height + threads_per_threadgroup.height - 1) / threads_per_threadgroup.height, 1);

	id<MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder];
	[compute_encoder setBuffer:(__bridge id<MTLBuffer>)constant_buf->impl._buffer offset:0 atIndex:0];
	[compute_encoder setAccelerationStructure:_instance_accel atBufferIndex:1];
	[compute_encoder setBuffer: (__bridge id<MTLBuffer>)ib[0]->impl.metal_buffer offset:0 atIndex:2];
	[compute_encoder setBuffer: (__bridge id<MTLBuffer>)vb[0]->impl.metal_buffer offset:0 atIndex:3];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)output->impl._tex atIndex:0];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texpaint0->impl._tex atIndex:1];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texpaint1->impl._tex atIndex:2];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texpaint2->impl._tex atIndex:3];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texenv->impl._tex atIndex:4];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texsobol->impl._tex atIndex:5];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texscramble->impl._tex atIndex:6];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texrank->impl._tex atIndex:7];

	for (id<MTLAccelerationStructure> primitive_accel in _primitive_accels) {
		[compute_encoder useResource:primitive_accel usage:MTLResourceUsageRead];
	}

	[compute_encoder setComputePipelineState:_raytracing_pipeline];
	[compute_encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threads_per_threadgroup];
	[compute_encoder endEncoding];
	[command_buffer commit];
}
