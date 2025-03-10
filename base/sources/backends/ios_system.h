#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreMotion/CMMotionManager.h>

struct iron_gpu_texture;

@interface GLView : UIView <UIKeyInput> {
@private
	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> commandEncoder;
	id<CAMetalDrawable> drawable;
	id<MTLLibrary> library;
	MTLRenderPassDescriptor *renderPassDescriptor;
	id<MTLTexture> depthTexture;

	CMMotionManager *motionManager;
	bool hasAccelerometer;
	float lastAccelerometerX, lastAccelerometerY, lastAccelerometerZ;
}

- (void)begin;
- (void)end;
- (void)showKeyboard;
- (void)hideKeyboard;
- (CAMetalLayer *)metalLayer;
- (id<MTLDevice>)metalDevice;
- (id<MTLLibrary>)metalLibrary;
- (id<MTLCommandQueue>)metalQueue;

@end

@interface GLViewController : UIViewController <UIDocumentPickerDelegate, UIDropInteractionDelegate> {
@private
}

- (void)loadView;

- (void)setVisible:(BOOL)value;

@end

@class GLView;

@interface IronAppDelegate : NSObject <UIApplicationDelegate> {
}

@end

@interface Motion : NSObject <UIAccelerometerDelegate> {

}

@end
