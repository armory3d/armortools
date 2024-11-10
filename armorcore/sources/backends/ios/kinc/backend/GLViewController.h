#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#ifndef KINC_TVOS
#import <CoreMotion/CMMotionManager.h>
#endif

@interface GLViewController : UIViewController <UIDocumentPickerDelegate, UIDropInteractionDelegate> {
@private
}

    - (void)loadView;

    - (void)setVisible:(BOOL)value;

    @end
