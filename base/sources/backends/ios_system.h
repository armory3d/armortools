#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreMotion/CMMotionManager.h>

@interface MyView : UIView <UIKeyInput> {
@private
	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> commandEncoder;
	id<CAMetalDrawable> drawable;
	id<MTLLibrary> library;
	MTLRenderPassDescriptor *renderPassDescriptor;
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
- (id<MTLCommandQueue>)metalQueue;
- (BOOL)hasText;
- (void)insertText:(NSString *)text;
- (void)deleteBackward;
@end

@interface MyViewController : UIViewController <UIDocumentPickerDelegate, UIDropInteractionDelegate> {}
- (void)loadView;
- (void)setVisible:(BOOL)value;
- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures;
@end


@interface IronSceneDelegate : UIResponder <UIWindowSceneDelegate> {}
@property (strong, nonatomic) UIWindow *window;
@end
