#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <CoreMotion/CMMotionManager.h>

@interface GLViewController : UIViewController <UIDocumentPickerDelegate, UIDropInteractionDelegate> {
@private
}

- (void)loadView;

- (void)setVisible:(BOOL)value;

@end
