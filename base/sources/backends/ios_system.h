#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreMotion/CMMotionManager.h>

@interface MyView : UIView <UIKeyInput> {
@private
	id<MTLDevice> device;
	id<MTLCommandQueue> queue;
	id<MTLLibrary> library;
}
- (void)show_keyboard;
- (void)hide_keyboard;
- (CAMetalLayer *)metal_layer;
- (id<MTLDevice>)metal_device;
- (id<MTLCommandQueue>)metal_queue;
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
