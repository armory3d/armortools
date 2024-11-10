#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#ifdef KINC_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#else
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#endif
#ifndef KINC_TVOS
#import <CoreMotion/CMMotionManager.h>
#endif

struct kinc_g5_render_target;

@interface GLView : UIView <UIKeyInput> {
@private
#ifdef KINC_METAL
	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> commandEncoder;
	id<CAMetalDrawable> drawable;
	id<MTLLibrary> library;
	MTLRenderPassDescriptor *renderPassDescriptor;
	id<MTLTexture> depthTexture;
#else
	EAGLContext *context;
	GLuint defaultFramebuffer, colorRenderbuffer, depthStencilRenderbuffer;
#endif

#ifndef KINC_TVOS
	CMMotionManager *motionManager;
#endif
	bool hasAccelerometer;
	float lastAccelerometerX, lastAccelerometerY, lastAccelerometerZ;
}

- (void)begin;
- (void)end;
- (void)showKeyboard;
- (void)hideKeyboard;
#ifdef KINC_METAL
- (CAMetalLayer *)metalLayer;
- (id<MTLDevice>)metalDevice;
- (id<MTLLibrary>)metalLibrary;
- (id<MTLCommandQueue>)metalQueue;
#endif

@end
