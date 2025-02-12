#include "metal.h"
#include <kinc/color.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/rendertarget.h>
#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>

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

static kinc_g5_render_target_t fallback_render_target;

id getMetalEncoder(void) {
	return render_command_encoder;
}

void kinc_g5_internal_destroy_window(int window) {}

void kinc_g5_internal_destroy(void) {}

extern void kinc_g4_on_g5_internal_resize(int, int, int);

void kinc_internal_resize(int window, int width, int height) {
	kinc_g4_on_g5_internal_resize(window, width, height);
}

void kinc_g5_internal_init(void) {}

void kinc_g5_internal_init_window(int window, int depthBufferBits, bool vsync) {
	depthBits = depthBufferBits;
	kinc_g5_render_target_init(&fallback_render_target, 32, 32, KINC_G5_RENDER_TARGET_FORMAT_32BIT, 0);
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

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {
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

void kinc_g5_end(int window) {}

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

void kinc_g5_internal_new_render_pass(kinc_g5_render_target_t **renderTargets, int count, bool wait, unsigned clear_flags, unsigned color, float depth) {
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

bool kinc_g5_supports_raytracing(void) {
	return false;
}

int kinc_g5_max_bound_textures(void) {
	return 16;
}
