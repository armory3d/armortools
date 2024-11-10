#import "GLView.h"

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/rendertarget.h>
#include <kinc/input/acceleration.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/input/pen.h>
#include <kinc/input/rotation.h>
#include <kinc/input/surface.h>
#include <kinc/system.h>

#ifdef KINC_OPENGL
#include <kinc/backend/graphics4/OpenGLWindow.h>
#endif

static const int touchmaxcount = 20;
static void *touches[touchmaxcount];

static void initTouches(void) {
	for (int i = 0; i < touchmaxcount; ++i) {
		touches[i] = NULL;
	}
}

static int getTouchIndex(void *touch) {
	for (int i = 0; i < touchmaxcount; ++i) {
		if (touches[i] == touch)
			return i;
	}
	return -1;
}

static int addTouch(void *touch) {
	for (int i = 0; i < touchmaxcount; ++i) {
		if (touches[i] == NULL) {
			touches[i] = touch;
			return i;
		}
	}
	return -1;
}

static int removeTouch(void *touch) {
	for (int i = 0; i < touchmaxcount; ++i) {
		if (touches[i] == touch) {
			touches[i] = NULL;
			return i;
		}
	}
	return -1;
}

static GLint backingWidth, backingHeight;

int kinc_window_width(int window) {
	return backingWidth;
}

int kinc_window_height(int window) {
	return backingHeight;
}

@implementation GLView

#ifdef KINC_METAL
+ (Class)layerClass {
	return [CAMetalLayer class];
}
#else
+ (Class)layerClass {
	return [CAEAGLLayer class];
}
#endif

#ifdef KINC_OPENGL
extern int kinc_ios_gl_framebuffer;
#endif

#ifdef KINC_METAL
- (void)hoverGesture:(UIHoverGestureRecognizer *)recognizer {
	CGPoint point = [recognizer locationInView:self];
	float x = point.x * self.contentScaleFactor;
	float y = point.y * self.contentScaleFactor;
	// Pencil hover
	kinc_internal_pen_trigger_move(0, x, y, 0.0);
}

- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:(CGRect)frame];
	self.contentScaleFactor = [UIScreen mainScreen].scale;

	backingWidth = frame.size.width * self.contentScaleFactor;
	backingHeight = frame.size.height * self.contentScaleFactor;

	initTouches();

	device = MTLCreateSystemDefaultDevice();
	commandQueue = [device newCommandQueue];
	library = [device newDefaultLibrary];

	CAMetalLayer *metalLayer = (CAMetalLayer *)self.layer;

	metalLayer.device = device;
	metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	metalLayer.framebufferOnly = YES;
	// metalLayer.presentsWithTransaction = YES;

	metalLayer.opaque = YES;
	metalLayer.backgroundColor = nil;

	[self addGestureRecognizer:[[UIHoverGestureRecognizer alloc] initWithTarget:self action:@selector(hoverGesture:)]];

	return self;
}
#else
- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:(CGRect)frame];
	self.contentScaleFactor = [UIScreen mainScreen].scale;

	backingWidth = frame.size.width * self.contentScaleFactor;
	backingHeight = frame.size.height * self.contentScaleFactor;

	initTouches();

	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
	                                                                          kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

	if (!context || ![EAGLContext setCurrentContext:context]) {
		//[self release];
		return nil;
	}

	glGenFramebuffersOES(1, &defaultFramebuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	kinc_ios_gl_framebuffer = defaultFramebuffer;

	glGenRenderbuffersOES(1, &colorRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);

	glGenRenderbuffersOES(1, &depthStencilRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthStencilRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthStencilRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthStencilRenderbuffer);

	// Start acceletometer
	hasAccelerometer = false;
#ifndef KINC_TVOS
	motionManager = [[CMMotionManager alloc] init];
	if ([motionManager isAccelerometerAvailable]) {
		motionManager.accelerometerUpdateInterval = 0.033;
		[motionManager startAccelerometerUpdates];
		hasAccelerometer = true;
	}
#endif

#ifndef KINC_TVOS
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onKeyboardHide:) name:UIKeyboardWillHideNotification object:nil];
#endif

	return self;
}
#endif

#ifdef KINC_METAL
- (void)begin {
}
#else
- (void)begin {
	[EAGLContext setCurrentContext:context];
	// glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	// glViewport(0, 0, backingWidth, backingHeight);

#ifndef KINC_TVOS
	// Accelerometer updates
	if (hasAccelerometer) {

		CMAcceleration acc = motionManager.accelerometerData.acceleration;

		if (acc.x != lastAccelerometerX || acc.y != lastAccelerometerY || acc.z != lastAccelerometerZ) {

			kinc_internal_on_acceleration(acc.x, acc.y, acc.z);

			lastAccelerometerX = acc.x;
			lastAccelerometerY = acc.y;
			lastAccelerometerZ = acc.z;
		}
	}
#endif
}
#endif

#ifdef KINC_METAL
- (void)end {
}
#else
- (void)end {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER_OES]; // crash at end
}
#endif

void kinc_internal_call_resize_callback(int window, int width, int height);

#ifdef KINC_METAL
- (void)layoutSubviews {
	backingWidth = self.frame.size.width * self.contentScaleFactor;
	backingHeight = self.frame.size.height * self.contentScaleFactor;

	CAMetalLayer *metalLayer = (CAMetalLayer *)self.layer;
	metalLayer.drawableSize = CGSizeMake(backingWidth, backingHeight);

	kinc_internal_call_resize_callback(0, backingWidth, backingHeight);
}
#else
- (void)layoutSubviews {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer];

	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);

	printf("backingWitdh/Height: %i, %i\n", backingWidth, backingHeight);

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthStencilRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH24_STENCIL8_OES, backingWidth, backingHeight);

	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
	}

	kinc_internal_call_resize_callback(0, backingWidth, backingHeight);
}
#endif

#ifdef KINC_METAL
- (void)dealloc {
}
#else
- (void)dealloc {
	if (defaultFramebuffer) {
		glDeleteFramebuffersOES(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}

	if (colorRenderbuffer) {
		glDeleteRenderbuffersOES(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}

	if (depthStencilRenderbuffer) {
		glDeleteRenderbuffersOES(1, &depthStencilRenderbuffer);
		depthStencilRenderbuffer = 0;
	}

	if ([EAGLContext currentContext] == context)
		[EAGLContext setCurrentContext:nil];

	//[context release];
	context = nil;

	//[super dealloc];
}
#endif

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		int index = getTouchIndex((__bridge void *)touch);
		if (index == -1)
			index = addTouch((__bridge void *)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_press(0, event.buttonMask == UIEventButtonMaskSecondary ? 1 : 0, x, y);
			}
			kinc_internal_surface_trigger_touch_start(index, x, y);

			if (touch.type == UITouchTypePencil) {
				kinc_internal_pen_trigger_press(0, x, y, 0.0);
			}
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		int index = getTouchIndex((__bridge void *)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_move(0, x, y);
			}
			kinc_internal_surface_trigger_move(index, x, y);
		}
	}
}

- (void)touchesEstimatedPropertiesUpdated:(NSSet<UITouch *> *)touches {
	for (UITouch *touch in touches) {
		if (touch.type == UITouchTypePencil) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			kinc_internal_pen_trigger_move(0, x, y, touch.force);
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		int index = removeTouch((__bridge void *)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_release(0, event.buttonMask == UIEventButtonMaskSecondary ? 1 : 0, x, y);
			}
			kinc_internal_surface_trigger_touch_end(index, x, y);

			if (touch.type == UITouchTypePencil) {
				kinc_internal_pen_trigger_release(0, x, y, 0.0);
			}
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	for (UITouch *touch in touches) {
		int index = removeTouch((__bridge void *)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_release(0, event.buttonMask == UIEventButtonMaskSecondary ? 1 : 0, x, y);
			}
			kinc_internal_surface_trigger_touch_end(index, x, y);

			if (touch.type == UITouchTypePencil) {
				kinc_internal_pen_trigger_release(0, x, y, 0.0);
			}
		}
	}
}

static NSString *keyboardstring;
static UITextField *myTextField = NULL;
static bool shiftDown = false;

- (void)showKeyboard {
	[self becomeFirstResponder];
}

- (void)hideKeyboard {
	[self resignFirstResponder];
}

- (BOOL)hasText {
	return YES;
}

- (void)insertText:(NSString *)text {
	if ([text length] == 1) {
		unichar ch = [text characterAtIndex:[text length] - 1];
		if (ch == 8212)
			ch = '_';
		if (ch == L'\n') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_RETURN);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_RETURN);
			return;
		}

		if (ch == L'.') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_PERIOD);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_PERIOD);
		}
		else if (ch == L'%') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_PERCENT);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_PERCENT);
		}
		else if (ch == L'(') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_OPEN_PAREN);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_OPEN_PAREN);
		}
		else if (ch == L'&') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_AMPERSAND);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_AMPERSAND);
		}
		else if (ch == L'$') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_DOLLAR);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_DOLLAR);
		}
		else if (ch == L'#') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_HASH);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_HASH);
		}
		else if (ch >= L'a' && ch <= L'z') {
			if (shiftDown) {
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SHIFT);
				shiftDown = false;
			}
			kinc_internal_keyboard_trigger_key_down(ch + KINC_KEY_A - L'a');
			kinc_internal_keyboard_trigger_key_up(ch + KINC_KEY_A - L'a');
		}
		else {
			if (!shiftDown) {
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SHIFT);
				shiftDown = true;
			}
			kinc_internal_keyboard_trigger_key_down(ch + KINC_KEY_A - L'A');
			kinc_internal_keyboard_trigger_key_up(ch + KINC_KEY_A - L'A');
		}

		kinc_internal_keyboard_trigger_key_press(ch);
	}
}

- (void)deleteBackward {
	kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACKSPACE);
	kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACKSPACE);
}

- (BOOL)canBecomeFirstResponder {
	return YES;
}

- (void)onKeyboardHide:(NSNotification *)notification {
	kinc_keyboard_hide();
}

#ifdef KINC_METAL
- (CAMetalLayer *)metalLayer {
	return (CAMetalLayer *)self.layer;
}

- (id<MTLDevice>)metalDevice {
	return device;
}

- (id<MTLLibrary>)metalLibrary {
	return library;
}

- (id<MTLCommandQueue>)metalQueue {
	return commandQueue;
}
#endif

@end
