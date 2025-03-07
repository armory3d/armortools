#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>
#include "metal.h"
#include <iron_math.h>
#include <iron_system.h>
#include <iron_gpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static id<MTLCommandBuffer> command_buffer = nil;
static id<MTLRenderCommandEncoder> render_command_encoder = nil;
static id<MTLComputeCommandEncoder> compute_command_encoder = nil;

static void start_render_pass(void);
static void end_render_pass(void);

id getMetalLayer(void);
id getMetalDevice(void);
id getMetalQueue(void);

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

id<CAMetalDrawable> drawable;
id<MTLTexture> depthTexture;
int depthBits;

static kinc_g5_texture_t fallback_render_target;

id getMetalEncoder(void) {
	return render_command_encoder;
}

void kinc_g5_internal_destroy_window() {}

void kinc_g5_internal_destroy(void) {}

void kinc_g5_internal_resize(int, int);

void kinc_internal_resize(int width, int height) {
	kinc_g5_internal_resize(width, height);
}

void kinc_g5_internal_init(void) {}

void kinc_g5_internal_init_window(int depthBufferBits, bool vsync) {
	depthBits = depthBufferBits;
	kinc_g5_render_target_init(&fallback_render_target, 32, 32, KINC_IMAGE_FORMAT_RGBA32, 0);
}

void kinc_g5_flush(void) {}

bool kinc_internal_metal_has_depth = false;

bool kinc_internal_current_render_target_has_depth(void) {
	return kinc_internal_metal_has_depth;
}

static void start_render_pass(void) {
	id<MTLTexture> texture = drawable.texture;
	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
	renderPassDescriptor.depthAttachment.clearDepth = 1;
	renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	renderPassDescriptor.depthAttachment.texture = depthTexture;
	renderPassDescriptor.stencilAttachment.clearStencil = 0;
	renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
	renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
	renderPassDescriptor.stencilAttachment.texture = depthTexture;

	render_command_encoder = [command_buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

static void end_render_pass(void) {
	[render_command_encoder endEncoding];
	render_command_encoder = nil;
}

void kinc_g5_begin(kinc_g5_texture_t *renderTarget) {
	CAMetalLayer *metalLayer = getMetalLayer();
	drawable = [metalLayer nextDrawable];

	if (depthBits > 0 && (depthTexture == nil || depthTexture.width != drawable.texture.width || depthTexture.height != drawable.texture.height)) {
		MTLTextureDescriptor *descriptor = [MTLTextureDescriptor new];
		descriptor.textureType = MTLTextureType2D;
		descriptor.width = drawable.texture.width;
		descriptor.height = drawable.texture.height;
		descriptor.depth = 1;
		descriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
		descriptor.arrayLength = 1;
		descriptor.mipmapLevelCount = 1;
		descriptor.resourceOptions = MTLResourceStorageModePrivate;
		descriptor.usage = MTLTextureUsageRenderTarget;
		id<MTLDevice> device = getMetalDevice();
		depthTexture = [device newTextureWithDescriptor:descriptor];
		kinc_internal_metal_has_depth = true;
	}
	else {
		kinc_internal_metal_has_depth = false;
	}

	id<MTLTexture> texture = drawable.texture;
	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	renderPassDescriptor.colorAttachments[0].texture = texture;
	renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
	renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
	renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
	renderPassDescriptor.depthAttachment.clearDepth = 1;
	renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
	renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	renderPassDescriptor.depthAttachment.texture = depthTexture;
	renderPassDescriptor.stencilAttachment.clearStencil = 0;
	renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
	renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
	renderPassDescriptor.stencilAttachment.texture = depthTexture;

	if (command_buffer != nil && render_command_encoder != nil) {
		[render_command_encoder endEncoding];
		[command_buffer commit];
	}

	id<MTLCommandQueue> commandQueue = getMetalQueue();
	command_buffer = [commandQueue commandBuffer];
	render_command_encoder = [command_buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

void kinc_g5_end() {}

bool kinc_g5_swap_buffers(void) {
	if (command_buffer != nil && render_command_encoder != nil) {
		[render_command_encoder endEncoding];
		[command_buffer presentDrawable:drawable];
		[command_buffer commit];
	}
	drawable = nil;
	command_buffer = nil;
	render_command_encoder = nil;

	return true;
}

void kinc_g5_internal_new_render_pass(kinc_g5_texture_t **renderTargets, int count, bool wait, unsigned clear_flags, unsigned color, float depth) {
	if (command_buffer != nil && render_command_encoder != nil) {
		[render_command_encoder endEncoding];
		[command_buffer commit];
		if (wait) {
			[command_buffer waitUntilCompleted];
		}
	}

	MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
	for (int i = 0; i < count; ++i) {
		if (renderTargets == NULL) {
			if (drawable == nil) {
				renderPassDescriptor.colorAttachments[i].texture = (__bridge id<MTLTexture>)fallback_render_target.impl._tex;
				renderPassDescriptor.depthAttachment.texture = nil;
				renderPassDescriptor.stencilAttachment.texture = nil;
				kinc_internal_metal_has_depth = false;
			}
			else {
				renderPassDescriptor.colorAttachments[i].texture = drawable.texture;
				renderPassDescriptor.depthAttachment.texture = depthTexture;
				renderPassDescriptor.stencilAttachment.texture = depthTexture;
				kinc_internal_metal_has_depth = depthTexture != nil;
			}
		}
		else {
			renderPassDescriptor.colorAttachments[i].texture = (__bridge id<MTLTexture>)renderTargets[i]->impl._tex;
			renderPassDescriptor.depthAttachment.texture = (__bridge id<MTLTexture>)renderTargets[0]->impl._depthTex;
			renderPassDescriptor.stencilAttachment.texture = (__bridge id<MTLTexture>)renderTargets[0]->impl._depthTex;
			kinc_internal_metal_has_depth = renderTargets[0]->impl._depthTex != nil;
		}
		if (clear_flags & KINC_G5_CLEAR_COLOR) {
			float red, green, blue, alpha;
			kinc_color_components(color, &red, &green, &blue, &alpha);
			renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
			renderPassDescriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
			renderPassDescriptor.colorAttachments[i].clearColor = MTLClearColorMake(red, green, blue, alpha);
		}
		else {
			renderPassDescriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
			renderPassDescriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
			renderPassDescriptor.colorAttachments[i].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
		}
	}

	if (clear_flags & KINC_G5_CLEAR_DEPTH) {
		renderPassDescriptor.depthAttachment.clearDepth = depth;
		renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
		renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	}
	else {
		renderPassDescriptor.depthAttachment.clearDepth = 1;
		renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
		renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
	}

	renderPassDescriptor.stencilAttachment.clearStencil = 0;
	renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
	renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;

	id<MTLCommandQueue> commandQueue = getMetalQueue();
	command_buffer = [commandQueue commandBuffer];
	render_command_encoder = [command_buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
}

bool kinc_g5_raytrace_supported(void) {
	return true;
}

int kinc_g5_max_bound_textures(void) {
	return 16;
}

id getMetalDevice(void);
id getMetalQueue(void);
id getMetalEncoder(void);

void kinc_g5_internal_new_render_pass(kinc_g5_texture_t **renderTargets, int count, bool wait, unsigned clear_flags, unsigned color, float depth);
void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline);

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {
	list->impl.current_index_buffer = NULL;
}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

static kinc_g5_texture_t *lastRenderTargets[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static kinc_g5_pipeline_t *lastPipeline = NULL;

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

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
	list->impl.current_index_buffer = NULL;
	lastRenderTargets[0] = NULL;
}

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget, unsigned flags, unsigned color, float depth) {
	if (renderTarget->framebuffer_index >= 0) {
		kinc_g5_internal_new_render_pass(NULL, 1, false, flags, color, depth);
	}
	else {
		kinc_g5_internal_new_render_pass(&renderTarget, 1, false, flags, color, depth);
	}
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}

void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, kinc_g5_index_buffer_count(list->impl.current_index_buffer));
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count) {
	id<MTLBuffer> indexBuffer = (__bridge id<MTLBuffer>)list->impl.current_index_buffer->impl.metal_buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
	                    indexCount:count
	                     indexType:MTLIndexTypeUInt32
	                   indexBuffer:indexBuffer
	             indexBufferOffset:start * 4];
}

void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	MTLViewport viewport;
	viewport.originX = x;
	viewport.originY = y;
	viewport.width = width;
	viewport.height = height;
	viewport.znear = 0.1;
	viewport.zfar = 100.0;
	[encoder setViewport:viewport];
}

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	MTLScissorRect scissor;
	scissor.x = x;
	scissor.y = y;
	int target_w = -1;
	int target_h = -1;
	if (lastRenderTargets[0] != NULL) {
		target_w = lastRenderTargets[0]->width;
		target_h = lastRenderTargets[0]->height;
	}
	else {
		target_w = kinc_window_width();
		target_h = kinc_window_height();
	}
	scissor.width = (x + width <= target_w) ? width : target_w - x;
	scissor.height = (y + height <= target_h) ? height : target_h - y;
	[encoder setScissorRect:scissor];
}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	MTLScissorRect scissor;
	scissor.x = 0;
	scissor.y = 0;
	if (lastRenderTargets[0] != NULL) {
		scissor.width = lastRenderTargets[0]->width;
		scissor.height = lastRenderTargets[0]->height;
	}
	else {
		scissor.width = kinc_window_width();
		scissor.height = kinc_window_height();
	}
	[encoder setScissorRect:scissor];
}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	kinc_g5_internal_pipeline_set(pipeline);
	lastPipeline = pipeline;
}

void kinc_g5_command_list_set_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {
	kinc_g5_internal_vertex_buffer_set(buffer);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	list->impl.current_index_buffer = buffer;
}

extern bool kinc_internal_metal_has_depth;

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_texture **targets, int count) {
	if (targets[0]->framebuffer_index >= 0) {
		for (int i = 0; i < 8; ++i)
			lastRenderTargets[i] = NULL;
		kinc_g5_internal_new_render_pass(NULL, 1, false, 0, 0, 0.0f);
	}
	else {
		for (int i = 0; i < count; ++i)
			lastRenderTargets[i] = targets[i];
		for (int i = count; i < 8; ++i)
			lastRenderTargets[i] = NULL;
		kinc_g5_internal_new_render_pass(targets, count, false, 0, 0, 0.0f);
	}
}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_texture_t *render_target, uint8_t *data) {
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

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	if (lastRenderTargets[0] == NULL) {
		kinc_g5_internal_new_render_pass(NULL, 1, false, 0, 0, 0.0f);
	}
	else {
		int count = 1;
		while (lastRenderTargets[count] != NULL)
			count++;
		kinc_g5_internal_new_render_pass(lastRenderTargets, count, false, 0, 0, 0.0f);
	}
	if (lastPipeline != NULL)
		kinc_g5_internal_pipeline_set(lastPipeline);
}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {
	id<MTLCommandQueue> commandQueue = getMetalQueue();
	id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];
}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setVertexBuffer:buf offset:offset atIndex:1];
}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentBuffer:buf offset:offset atIndex:0];
}

void kinc_g5_command_list_set_compute_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {
	assert(compute_command_encoder != nil);
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	[compute_command_encoder setBuffer:buf offset:offset atIndex:1];
}

void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {
}

void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._tex;
	if (compute_command_encoder != nil) {
		[compute_command_encoder setTexture:tex atIndex:unit.stages[KINC_G5_SHADER_TYPE_COMPUTE]];
	}
	else {
		if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
			[render_command_encoder setVertexTexture:tex atIndex:unit.stages[KINC_G5_SHADER_TYPE_VERTEX]];
		}
		if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
			[render_command_encoder setFragmentTexture:tex atIndex:unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]];
		}
	}
}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *target) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	id<MTLTexture> depth_tex = (__bridge id<MTLTexture>)target->impl._depthTex;
	if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		[encoder setVertexTexture:depth_tex atIndex:unit.stages[KINC_G5_SHADER_TYPE_VERTEX]];
	}
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		[encoder setFragmentTexture:depth_tex atIndex:unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]];
	}
}

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	id<MTLSamplerState> mtl_sampler = (__bridge id<MTLSamplerState>)sampler->impl.sampler;

	if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		[encoder setVertexSamplerState:mtl_sampler atIndex:unit.stages[KINC_G5_SHADER_TYPE_VERTEX]];
	}
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		[encoder setFragmentSamplerState:mtl_sampler atIndex:unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]];
	}
}

void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, kinc_g5_compute_shader *shader) {
	if (compute_command_encoder == nil) {
		end_render_pass();
		compute_command_encoder = [command_buffer computeCommandEncoder];
	}

	id<MTLComputePipelineState> pipeline = (__bridge id<MTLComputePipelineState>)shader->impl._pipeline;
	[compute_command_encoder setComputePipelineState:pipeline];
}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {
	assert(compute_command_encoder != nil);

	MTLSize perGrid;
	perGrid.width = x;
	perGrid.height = y;
	perGrid.depth = z;
	MTLSize perGroup;
	perGroup.width = 16;
	perGroup.height = 16;
	perGroup.depth = 1;
	[compute_command_encoder dispatchThreadgroups:perGrid threadsPerThreadgroup:perGroup];

	[compute_command_encoder endEncoding];

	compute_command_encoder = nil;

	start_render_pass();
}

id getMetalDevice(void);
id getMetalLibrary(void);

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *_data, int length) {
	shader->impl.name[0] = 0;

	{
		uint8_t *data = (uint8_t *)_data;
		if (length > 1 && data[0] == '>') {
			memcpy(shader->impl.name, data + 1, length - 1);
			shader->impl.name[length - 1] = 0;
		}
		else {
			for (int i = 3; i < length; ++i) {
				if (data[i] == '\n') {
					shader->impl.name[i - 3] = 0;
					break;
				}
				else {
					shader->impl.name[i - 3] = data[i];
				}
			}
		}
	}

	char *data = (char *)_data;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:nil];
	}
	id<MTLFunction> function = [library newFunctionWithName:[NSString stringWithCString:shader->impl.name encoding:NSUTF8StringEncoding]];
	assert(function != nil);
	shader->impl._function = (__bridge_retained void *)function;

	id<MTLDevice> device = getMetalDevice();
	MTLComputePipelineReflection *reflection = nil;
	NSError *error = nil;
	shader->impl._pipeline = (__bridge_retained void *)[device newComputePipelineStateWithFunction:function
	                                                                                       options:MTLPipelineOptionBufferTypeInfo
	                                                                                    reflection:&reflection
	                                                                                         error:&error];
	if (error != nil)
		NSLog(@"%@", [error localizedDescription]);
	assert(shader->impl._pipeline != NULL && !error);
	shader->impl._reflection = (__bridge_retained void *)reflection;
}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {
	id<MTLFunction> function = (__bridge_transfer id<MTLFunction>)shader->impl._function;
	function = nil;
	shader->impl._function = NULL;

	id<MTLComputePipelineState> pipeline = (__bridge_transfer id<MTLComputePipelineState>)shader->impl._pipeline;
	pipeline = nil;
	shader->impl._pipeline = NULL;

	MTLComputePipelineReflection *reflection = (__bridge_transfer MTLComputePipelineReflection *)shader->impl._reflection;
	reflection = nil;
	shader->impl._reflection = NULL;
}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;
	location.impl.computeOffset = -1;

	MTLComputePipelineReflection *reflection = (__bridge MTLComputePipelineReflection *)shader->impl._reflection;

	for (MTLArgument *arg in reflection.arguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.computeOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	MTLComputePipelineReflection *reflection = (__bridge MTLComputePipelineReflection *)shader->impl._reflection;
	for (MTLArgument *arg in reflection.arguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.stages[KINC_G5_SHADER_TYPE_COMPUTE] = (int)[arg index];
		}
	}

	return unit;
}

id getMetalDevice(void);
id getMetalEncoder(void);

static MTLBlendFactor convert_blending_factor(kinc_g5_blending_factor_t factor) {
	switch (factor) {
	case KINC_G5_BLEND_ONE:
		return MTLBlendFactorOne;
	case KINC_G5_BLEND_ZERO:
		return MTLBlendFactorZero;
	case KINC_G5_BLEND_SOURCE_ALPHA:
		return MTLBlendFactorSourceAlpha;
	case KINC_G5_BLEND_DEST_ALPHA:
		return MTLBlendFactorDestinationAlpha;
	case KINC_G5_BLEND_INV_SOURCE_ALPHA:
		return MTLBlendFactorOneMinusSourceAlpha;
	case KINC_G5_BLEND_INV_DEST_ALPHA:
		return MTLBlendFactorOneMinusDestinationAlpha;
	case KINC_G5_BLEND_SOURCE_COLOR:
		return MTLBlendFactorSourceColor;
	case KINC_G5_BLEND_DEST_COLOR:
		return MTLBlendFactorDestinationColor;
	case KINC_G5_BLEND_INV_SOURCE_COLOR:
		return MTLBlendFactorOneMinusSourceColor;
	case KINC_G5_BLEND_INV_DEST_COLOR:
		return MTLBlendFactorOneMinusDestinationColor;
	case KINC_G5_BLEND_CONSTANT:
		return MTLBlendFactorBlendColor;
	case KINC_G5_BLEND_INV_CONSTANT:
		return MTLBlendFactorOneMinusBlendColor;
	}
}

static MTLBlendOperation convert_blending_operation(kinc_g5_blending_operation_t op) {
	switch (op) {
	case KINC_G5_BLENDOP_ADD:
		return MTLBlendOperationAdd;
	case KINC_G5_BLENDOP_SUBTRACT:
		return MTLBlendOperationSubtract;
	case KINC_G5_BLENDOP_REVERSE_SUBTRACT:
		return MTLBlendOperationReverseSubtract;
	case KINC_G5_BLENDOP_MIN:
		return MTLBlendOperationMin;
	case KINC_G5_BLENDOP_MAX:
		return MTLBlendOperationMax;
	}
}

static MTLCompareFunction convert_compare_mode(kinc_g5_compare_mode_t compare) {
	switch (compare) {
	case KINC_G5_COMPARE_MODE_ALWAYS:
		return MTLCompareFunctionAlways;
	case KINC_G5_COMPARE_MODE_NEVER:
		return MTLCompareFunctionNever;
	case KINC_G5_COMPARE_MODE_EQUAL:
		return MTLCompareFunctionEqual;
	case KINC_G5_COMPARE_MODE_NOT_EQUAL:
		return MTLCompareFunctionNotEqual;
	case KINC_G5_COMPARE_MODE_LESS:
		return MTLCompareFunctionLess;
	case KINC_G5_COMPARE_MODE_LESS_EQUAL:
		return MTLCompareFunctionLessEqual;
	case KINC_G5_COMPARE_MODE_GREATER:
		return MTLCompareFunctionGreater;
	case KINC_G5_COMPARE_MODE_GREATER_EQUAL:
		return MTLCompareFunctionGreaterEqual;
	}
}

static MTLCullMode convert_cull_mode(kinc_g5_cull_mode_t cull) {
	switch (cull) {
	case KINC_G5_CULL_MODE_CLOCKWISE:
		return MTLCullModeFront;
	case KINC_G5_CULL_MODE_COUNTERCLOCKWISE:
		return MTLCullModeBack;
	case KINC_G5_CULL_MODE_NEVER:
		return MTLCullModeNone;
	}
}

static MTLPixelFormat convert_render_target_format(kinc_image_format_t format) {
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

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	memset(&pipeline->impl, 0, sizeof(pipeline->impl));
	kinc_g5_internal_pipeline_set_defaults(pipeline);
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline) {
	pipeline->impl._reflection = NULL;
	pipeline->impl._depthStencil = NULL;

	id<MTLRenderPipelineState> pipe = (__bridge_transfer id<MTLRenderPipelineState>)pipeline->impl._pipeline;
	pipe = nil;
	pipeline->impl._pipeline = NULL;

	MTLRenderPipelineReflection *reflection = (__bridge_transfer MTLRenderPipelineReflection *)pipeline->impl._reflection;
	reflection = nil;
	pipeline->impl._reflection = NULL;

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

static int findAttributeIndex(NSArray<MTLVertexAttribute *> *attributes, const char *name) {
	for (MTLVertexAttribute *attribute in attributes) {
		if (strcmp(name, [[attribute name] UTF8String]) == 0) {
			return (int)[attribute attributeIndex];
		}
	}
	return -1;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline) {
	MTLRenderPipelineDescriptor *renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = (__bridge id<MTLFunction>)pipeline->vertex_shader->impl.mtlFunction;
	renderPipelineDesc.fragmentFunction = (__bridge id<MTLFunction>)pipeline->fragment_shader->impl.mtlFunction;
	for (int i = 0; i < pipeline->color_attachment_count; ++i) {
		renderPipelineDesc.colorAttachments[i].pixelFormat = convert_render_target_format(pipeline->color_attachment[i]);
		renderPipelineDesc.colorAttachments[i].blendingEnabled =
		    pipeline->blend_source != KINC_G5_BLEND_ONE || pipeline->blend_destination != KINC_G5_BLEND_ZERO ||
		    pipeline->alpha_blend_source != KINC_G5_BLEND_ONE || pipeline->alpha_blend_destination != KINC_G5_BLEND_ZERO;
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
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;

	float offset = 0;
	MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];

	for (int i = 0; i < pipeline->input_layout->size; ++i) {
		int index = findAttributeIndex(renderPipelineDesc.vertexFunction.vertexAttributes, pipeline->input_layout->elements[i].name);

		if (index < 0) {
			kinc_log("Could not find vertex attribute %s\n", pipeline->input_layout->elements[i].name);
		}

		if (index >= 0) {
			vertexDescriptor.attributes[index].bufferIndex = 0;
			vertexDescriptor.attributes[index].offset = offset;
		}

		offset += kinc_g5_vertex_data_size(pipeline->input_layout->elements[i].data);
		if (index >= 0) {
			switch (pipeline->input_layout->elements[i].data) {
			case KINC_G5_VERTEX_DATA_F32_1X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat;
				break;
			case KINC_G5_VERTEX_DATA_F32_2X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat2;
				break;
			case KINC_G5_VERTEX_DATA_F32_3X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat3;
				break;
			case KINC_G5_VERTEX_DATA_F32_4X:
				vertexDescriptor.attributes[index].format = MTLVertexFormatFloat4;
				break;
			case KINC_G5_VERTEX_DATA_U8_4X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatUChar4Normalized;
				break;
			case KINC_G5_VERTEX_DATA_I16_2X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort2Normalized;
				break;
			case KINC_G5_VERTEX_DATA_I16_4X_NORMALIZED:
				vertexDescriptor.attributes[index].format = MTLVertexFormatShort4Normalized;
				break;
			}
		}
	}

	vertexDescriptor.layouts[0].stride = offset;
	vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	renderPipelineDesc.vertexDescriptor = vertexDescriptor;

	NSError *errors = nil;
	MTLRenderPipelineReflection *reflection = nil;
	id<MTLDevice> device = getMetalDevice();

	pipeline->impl._pipeline = (__bridge_retained void *)[device newRenderPipelineStateWithDescriptor:renderPipelineDesc
	                                                                                          options:MTLPipelineOptionBufferTypeInfo
	                                                                                       reflection:&reflection
	                                                                                            error:&errors];
	if (errors != nil)
		NSLog(@"%@", [errors localizedDescription]);
	assert(pipeline->impl._pipeline && !errors);

	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	pipeline->impl._pipelineDepth = (__bridge_retained void *)[device newRenderPipelineStateWithDescriptor:renderPipelineDesc
	                                                                                               options:MTLPipelineOptionBufferTypeInfo
	                                                                                            reflection:&reflection
	                                                                                                 error:&errors];
	if (errors != nil)
		NSLog(@"%@", [errors localizedDescription]);
	assert(pipeline->impl._pipelineDepth && !errors);

	pipeline->impl._reflection = (__bridge_retained void *)reflection;

	MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
	depthStencilDescriptor.depthCompareFunction = convert_compare_mode(pipeline->depth_mode);
	depthStencilDescriptor.depthWriteEnabled = pipeline->depth_write;
	pipeline->impl._depthStencil = (__bridge_retained void *)[device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

	depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
	depthStencilDescriptor.depthWriteEnabled = false;
	pipeline->impl._depthStencilNone = (__bridge_retained void *)[device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

bool kinc_internal_current_render_target_has_depth(void);

void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	if (kinc_internal_current_render_target_has_depth()) {
		id<MTLRenderPipelineState> pipe = (__bridge id<MTLRenderPipelineState>)pipeline->impl._pipelineDepth;
		[encoder setRenderPipelineState:pipe];
		id<MTLDepthStencilState> depthStencil = (__bridge id<MTLDepthStencilState>)pipeline->impl._depthStencil;
		[encoder setDepthStencilState:depthStencil];
	}
	else {
		id<MTLRenderPipelineState> pipe = (__bridge id<MTLRenderPipelineState>)pipeline->impl._pipeline;
		[encoder setRenderPipelineState:pipe];
		id<MTLDepthStencilState> depthStencil = (__bridge id<MTLDepthStencilState>)pipeline->impl._depthStencilNone;
		[encoder setDepthStencilState:depthStencil];
	}
	[encoder setFrontFacingWinding:MTLWindingClockwise];
	[encoder setCullMode:convert_cull_mode(pipeline->cull_mode)];
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name) {
	if (strcmp(name, "bias") == 0) {
		name = "bias0";
	}

	kinc_g5_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;
	location.impl.computeOffset = -1;

	MTLRenderPipelineReflection *reflection = (__bridge MTLRenderPipelineReflection *)pipeline->impl._reflection;

	for (MTLArgument *arg in reflection.vertexArguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.vertexOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	for (MTLArgument *arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeBuffer && [[arg name] isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.fragmentOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name) {
	kinc_g5_texture_unit_t unit = {0};
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	MTLRenderPipelineReflection *reflection = (__bridge MTLRenderPipelineReflection *)pipeline->impl._reflection;
	for (MTLArgument *arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] = (int)[arg index];
			break;
		}
	}

	for (MTLArgument *arg in reflection.vertexArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.stages[KINC_G5_SHADER_TYPE_VERTEX] = (int)[arg index];
			break;
		}
	}

	return unit;
}

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

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {
	id<MTLFunction> function = (__bridge_transfer id<MTLFunction>)shader->impl.mtlFunction;
	function = nil;
	shader->impl.mtlFunction = NULL;
}

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
	shader->impl.name[0] = 0;

	{
		uint8_t *data = (uint8_t *)source;
		if (length > 1 && data[0] == '>') {
			memcpy(shader->impl.name, data + 1, length - 1);
			shader->impl.name[length - 1] = 0;
		}
		else {
			for (int i = 3; i < length; ++i) {
				if (data[i] == '\n') {
					shader->impl.name[i - 3] = 0;
					break;
				}
				else {
					shader->impl.name[i - 3] = data[i];
				}
			}
		}
	}

	char *data = (char *)source;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		NSError *error = nil;
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:&error];
		if (library == nil) {
			kinc_error("%s", error.localizedDescription.UTF8String);
		}
	}
	shader->impl.mtlFunction = (__bridge_retained void *)[library newFunctionWithName:[NSString stringWithCString:shader->impl.name
	                                                                                                     encoding:NSUTF8StringEncoding]];

	assert(shader->impl.mtlFunction);
}


static kinc_g5_raytrace_acceleration_structure_t *accel;
static kinc_g5_raytrace_pipeline_t *pipeline;
static kinc_g5_texture_t *output = NULL;
static kinc_g5_constant_buffer_t *constant_buf;

id getMetalDevice(void);
id getMetalQueue(void);

typedef struct inst {
	kinc_matrix4x4_t m;
	int i;
} inst_t;

id<MTLComputePipelineState> _raytracing_pipeline;
NSMutableArray *_primitive_accels;
id<MTLAccelerationStructure> _instance_accel;
dispatch_semaphore_t _sem;

static kinc_g5_texture_t *_texpaint0;
static kinc_g5_texture_t *_texpaint1;
static kinc_g5_texture_t *_texpaint2;
static kinc_g5_texture_t *_texenv;
static kinc_g5_texture_t *_texsobol;
static kinc_g5_texture_t *_texscramble;
static kinc_g5_texture_t *_texrank;

static kinc_g5_vertex_buffer_t *vb[16];
static kinc_g5_vertex_buffer_t *vb_last[16];
static kinc_g5_index_buffer_t *ib[16];
static int vb_count = 0;
static int vb_count_last = 0;
static inst_t instances[1024];
static int instances_count = 0;

bool kinc_g5_raytrace_supported() {
	id<MTLDevice> device = getMetalDevice();
	return device.supportsRaytracing;
}

void kinc_g5_raytrace_pipeline_init(kinc_g5_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 kinc_g5_constant_buffer_t *constant_buffer) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
	constant_buf = constant_buffer;

	NSError *error = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:ray_shader length:ray_shader_size encoding:NSUTF8StringEncoding]
	                                              options:nil
	                                                error:&error];
	if (library == nil) {
		kinc_error("%s", error.localizedDescription.UTF8String);
	}

	MTLComputePipelineDescriptor *descriptor = [[MTLComputePipelineDescriptor alloc] init];
	descriptor.computeFunction = [library newFunctionWithName:@"raytracingKernel"];
	descriptor.threadGroupSizeIsMultipleOfThreadExecutionWidth = YES;
	_raytracing_pipeline = [device newComputePipelineStateWithDescriptor:descriptor options:0 reflection:nil error:&error];
	_sem = dispatch_semaphore_create(2);
}

void kinc_g5_raytrace_pipeline_destroy(kinc_g5_raytrace_pipeline_t *pipeline) {}

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

void kinc_g5_raytrace_acceleration_structure_init(kinc_g5_raytrace_acceleration_structure_t *accel) {
	vb_count = 0;
	instances_count = 0;
}

void kinc_g5_raytrace_acceleration_structure_add(kinc_g5_raytrace_acceleration_structure_t *accel, kinc_g5_vertex_buffer_t *_vb, kinc_g5_index_buffer_t *_ib,
	kinc_matrix4x4_t _transform) {

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

void _kinc_g5_raytrace_acceleration_structure_destroy_bottom(kinc_g5_raytrace_acceleration_structure_t *accel) {
//	for (int i = 0; i < vb_count_last; ++i) {
//	}
	_primitive_accels = nil;
}

void _kinc_g5_raytrace_acceleration_structure_destroy_top(kinc_g5_raytrace_acceleration_structure_t *accel) {
	_instance_accel = nil;
}

void kinc_g5_raytrace_acceleration_structure_build(kinc_g5_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list,
	kinc_g5_vertex_buffer_t *_vb_full, kinc_g5_index_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_kinc_g5_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_kinc_g5_raytrace_acceleration_structure_destroy_top(accel);
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
	descriptor.vertexBuffer = (__bridge id<MTLBuffer>)vb[0]->impl.mtlBuffer;
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

void kinc_g5_raytrace_acceleration_structure_destroy(kinc_g5_raytrace_acceleration_structure_t *accel) {}

void kinc_g5_raytrace_set_textures(kinc_g5_texture_t *texpaint0, kinc_g5_texture_t *texpaint1, kinc_g5_texture_t *texpaint2, kinc_g5_texture_t *texenv, kinc_g5_texture_t *texsobol, kinc_g5_texture_t *texscramble, kinc_g5_texture_t *texrank) {
	_texpaint0 = texpaint0;
	_texpaint1 = texpaint1;
	_texpaint2 = texpaint2;
	_texenv = texenv;
	_texsobol = texsobol;
	_texscramble = texscramble;
	_texrank = texrank;
}

void kinc_g5_raytrace_set_acceleration_structure(kinc_g5_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_g5_raytrace_set_pipeline(kinc_g5_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_g5_raytrace_set_target(kinc_g5_texture_t *_output) {
	output = _output;
}

void kinc_g5_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
	dispatch_semaphore_wait(_sem, DISPATCH_TIME_FOREVER);

	id<MTLCommandQueue> queue = getMetalQueue();
	id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
	__block dispatch_semaphore_t sem = _sem;
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
	[compute_encoder setBuffer: (__bridge id<MTLBuffer>)vb[0]->impl.mtlBuffer offset:0 atIndex:3];
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
	texture->data = NULL;
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

void kinc_g5_texture_destroy(kinc_g5_texture_t *target) {
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

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	switch (texture->format) {
	case KINC_IMAGE_FORMAT_R8:
		return texture->width;
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_BGRA32:
	default:
		return texture->width * 4;
	case KINC_IMAGE_FORMAT_RGBA64:
		return texture->width * 8;
	case KINC_IMAGE_FORMAT_RGBA128:
		return texture->width * 16;
	}
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

static void render_target_init(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits, int framebuffer_index) {
	memset(target, 0, sizeof(kinc_g5_texture_t));

	target->width = width;
	target->height = height;
	target->data = NULL;

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

void kinc_g5_render_target_init(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, -1);
	target->width = target->width = width;
	target->height = target->height = height;
	target->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
	target->_uploaded = true;
}

static int framebuffer_count = 0;

void kinc_g5_render_target_init_framebuffer(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, framebuffer_count);
	framebuffer_count += 1;
}

void kinc_g5_render_target_set_depth_from(kinc_g5_texture_t *target, kinc_g5_texture_t *source) {
	target->impl._depthTex = source->impl._depthTex;
}


id getMetalDevice(void);
id getMetalEncoder(void);

kinc_g5_vertex_buffer_t *currentVertexBuffer = NULL;

static void vertex_buffer_unset(kinc_g5_vertex_buffer_t *buffer) {
	if (currentVertexBuffer == buffer)
		currentVertexBuffer = NULL;
}

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	memset(&buffer->impl, 0, sizeof(buffer->impl));
	buffer->impl.myCount = count;
	buffer->impl.gpuMemory = gpuMemory;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += kinc_g5_vertex_data_size(element.data);
	}

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
	options |= MTLResourceStorageModeShared;

	id<MTLBuffer> buf = [device newBufferWithLength:count * buffer->impl.myStride options:options];
	buffer->impl.mtlBuffer = (__bridge_retained void *)buf;

	buffer->impl.lastStart = 0;
	buffer->impl.lastCount = 0;
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buf) {
	id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)buf->impl.mtlBuffer;
	buffer = nil;
	buf->impl.mtlBuffer = NULL;
	vertex_buffer_unset(buf);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buf) {
	buf->impl.lastStart = 0;
	buf->impl.lastCount = kinc_g5_vertex_buffer_count(buf);
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.mtlBuffer;
	float *floats = (float *)[buffer contents];
	return floats;
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buf, int start, int count) {
	buf->impl.lastStart = start;
	buf->impl.lastCount = count;
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.mtlBuffer;
	float *floats = (float *)[buffer contents];
	return &floats[start * buf->impl.myStride / sizeof(float)];
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buf) {
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buf, int count) {
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buf) {
	currentVertexBuffer = buf;

	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.mtlBuffer;
	[encoder setVertexBuffer:buffer offset:0 atIndex:0];

	return 0;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

bool kinc_g5_transpose_mat = true;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;
	buffer->impl._buffer = (__bridge_retained void *)[getMetalDevice() newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl._buffer;
	buf = nil;
	buffer->impl._buffer = NULL;
}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	uint8_t *data = (uint8_t *)[buf contents];
	buffer->data = &data[start];
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	buffer->data = NULL;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int indexCount, bool gpuMemory) {
	buffer->impl.count = indexCount;
	buffer->impl.gpu_memory = gpuMemory;
	buffer->impl.last_start = 0;
	buffer->impl.last_count = indexCount;

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
	options |= MTLResourceStorageModeShared;

	buffer->impl.metal_buffer = (__bridge_retained void *)[device
	    newBufferWithLength:sizeof(uint32_t) * indexCount
	                options:options];
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl.metal_buffer;
	buf = nil;
	buffer->impl.metal_buffer = NULL;
}

static int kinc_g5_internal_index_buffer_stride(kinc_g5_index_buffer_t *buffer) {
	return 4;
}

void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer) {
	return kinc_g5_index_buffer_lock(buffer, 0, kinc_g5_index_buffer_count(buffer));
}

void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count) {
	buffer->impl.last_start = start;
	buffer->impl.last_count = count;

	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->impl.metal_buffer;
	uint8_t *data = (uint8_t *)[metal_buffer contents];
	return &data[start * kinc_g5_internal_index_buffer_stride(buffer)];
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_index_buffer_unlock(buffer, buffer->impl.last_count);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
