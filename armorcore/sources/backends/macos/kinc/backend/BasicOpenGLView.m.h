#import "BasicOpenGLView.h"

#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/input/pen.h>
#include <kinc/system.h>

#ifdef KINC_METAL
#include <kinc/graphics5/graphics.h>
#endif

@implementation BasicOpenGLView

static bool shift = false;
static bool ctrl = false;
static bool alt = false;
static bool cmd = false;

#ifndef KINC_METAL
+ (NSOpenGLPixelFormat *)basicPixelFormat {
	// TODO (DK) pass via argument in
	int aa = 1; // Kore::Application::the()->antialiasing();
	if (aa > 0) {
		NSOpenGLPixelFormatAttribute attributes[] = {NSOpenGLPFADoubleBuffer,          NSOpenGLPFADepthSize,
		                                             (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
		                                             NSOpenGLPFAOpenGLProfile,         NSOpenGLProfileVersion3_2Core,
		                                             NSOpenGLPFASupersample,           NSOpenGLPFASampleBuffers,
		                                             (NSOpenGLPixelFormatAttribute)1,  NSOpenGLPFASamples,
		                                             (NSOpenGLPixelFormatAttribute)aa, NSOpenGLPFAStencilSize,
		                                             (NSOpenGLPixelFormatAttribute)8,  (NSOpenGLPixelFormatAttribute)0};
		return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	}
	else {
		NSOpenGLPixelFormatAttribute attributes[] = {
		    NSOpenGLPFADoubleBuffer,         NSOpenGLPFADepthSize,           (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
		    NSOpenGLPFAOpenGLProfile,        NSOpenGLProfileVersion3_2Core,  NSOpenGLPFAStencilSize,
		    (NSOpenGLPixelFormatAttribute)8, (NSOpenGLPixelFormatAttribute)0};
		return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	}
}

- (void)switchBuffers {
	[[self openGLContext] flushBuffer];
}
#endif

- (void)flagsChanged:(NSEvent *)theEvent {
	if (shift) {
		kinc_internal_keyboard_trigger_key_up(KINC_KEY_SHIFT);
		shift = false;
	}
	if (ctrl) {
		kinc_internal_keyboard_trigger_key_up(KINC_KEY_CONTROL);
		ctrl = false;
	}
	if (alt) {
		kinc_internal_keyboard_trigger_key_up(KINC_KEY_ALT);
		alt = false;
	}
	if (cmd) {
		kinc_internal_keyboard_trigger_key_up(KINC_KEY_META);
		cmd = false;
	}

	if ([theEvent modifierFlags] & NSShiftKeyMask) {
		kinc_internal_keyboard_trigger_key_down(KINC_KEY_SHIFT);
		shift = true;
	}
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		kinc_internal_keyboard_trigger_key_down(KINC_KEY_CONTROL);
		ctrl = true;
	}
	if ([theEvent modifierFlags] & NSAlternateKeyMask) {
		kinc_internal_keyboard_trigger_key_down(KINC_KEY_ALT);
		alt = true;
	}
	if ([theEvent modifierFlags] & NSCommandKeyMask) {
		kinc_internal_keyboard_trigger_key_down(KINC_KEY_META);
		cmd = true;
	}
}

- (void)keyDown:(NSEvent *)theEvent {
	if ([theEvent isARepeat])
		return;
	NSString *characters = [theEvent charactersIgnoringModifiers];
	if ([characters length]) {
		unichar ch = [characters characterAtIndex:0];
		switch (ch) { // keys that exist in keydown and keypress events
		case 59:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_SEMICOLON);
			break;
		case 91:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_OPEN_BRACKET);
			break;
		case 93:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_CLOSE_BRACKET);
			break;
		case 39:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_QUOTE);
			break;
		case 92:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACK_SLASH);
			break;
		case 44:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_COMMA);
			break;
		case 46:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_PERIOD);
			break;
		case 47:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_SLASH);
			break;
		case 96:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACK_QUOTE);
			break;
		case 32:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_SPACE);
			break;
		case 45: // we need breaks because EQUALS triggered too for some reason
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_HYPHEN_MINUS);
			break;
		case 61:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_EQUALS);
			break;
		}
		switch (ch) {
		case NSRightArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_RIGHT);
			break;
		case NSLeftArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_LEFT);
			break;
		case NSUpArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_UP);
			break;
		case NSDownArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_DOWN);
			break;
		case 27:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_ESCAPE);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_RETURN);
			kinc_internal_keyboard_trigger_key_press('\n');
			break;
		case 0x7f:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACKSPACE);
			kinc_internal_keyboard_trigger_key_press('\x08');
			break;
		case 9:
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_TAB);
			kinc_internal_keyboard_trigger_key_press('\t');
			break;
		default:
			if (ch == 'x' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char *text = kinc_internal_cut_callback();
				if (text != NULL) {
					NSPasteboard *board = [NSPasteboard generalPasteboard];
					[board clearContents];
					[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
				}
			}
			if (ch == 'c' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char *text = kinc_internal_copy_callback();
				if (text != NULL) {
					NSPasteboard *board = [NSPasteboard generalPasteboard];
					[board clearContents];
					[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
				}
			}
			if (ch == 'v' && [theEvent modifierFlags] & NSCommandKeyMask) {
				NSPasteboard *board = [NSPasteboard generalPasteboard];
				NSString *data = [board stringForType:NSStringPboardType];
				if (data != nil) {
					char charData[4096];
					strcpy(charData, [data UTF8String]);
					kinc_internal_paste_callback(charData);
				}
			}
			if (ch >= L'a' && ch <= L'z') {
				kinc_internal_keyboard_trigger_key_down(ch - L'a' + KINC_KEY_A);
			}
			else if (ch >= L'A' && ch <= L'Z') {
				kinc_internal_keyboard_trigger_key_down(ch - L'A' + KINC_KEY_A);
			}
			else if (ch >= L'0' && ch <= L'9') {
				kinc_internal_keyboard_trigger_key_down(ch - L'0' + KINC_KEY_0);
			}
			kinc_internal_keyboard_trigger_key_press(ch);
			break;
		}
	}
}

- (void)keyUp:(NSEvent *)theEvent {
	NSString *characters = [theEvent charactersIgnoringModifiers];
	if ([characters length]) {
		unichar ch = [characters characterAtIndex:0];
		switch (ch) {
		case 59:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_SEMICOLON);
			break;
		case 91:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_OPEN_BRACKET);
			break;
		case 93:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_CLOSE_BRACKET);
			break;
		case 39:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_QUOTE);
			break;
		case 92:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACK_SLASH);
			break;
		case 44:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_COMMA);
			break;
		case 46:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_PERIOD);
			break;
		case 47:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_SLASH);
			break;
		case 96:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACK_QUOTE);
			break;
		case 45:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_HYPHEN_MINUS);
			break;
		case 61:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_EQUALS);
			break;
		case NSRightArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_RIGHT);
			break;
		case NSLeftArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_LEFT);
			break;
		case NSUpArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_UP);
			break;
		case NSDownArrowFunctionKey:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_DOWN);
			break;
		case 27:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_ESCAPE);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_RETURN);
			break;
		case 0x7f:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACKSPACE);
			break;
		case 9:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_TAB);
			break;
		case 32:
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_SPACE);
			break;
		default:
			if (ch >= L'a' && ch <= L'z') {
				kinc_internal_keyboard_trigger_key_up(ch - L'a' + KINC_KEY_A);
			}
			else if (ch >= L'A' && ch <= L'Z') {
				kinc_internal_keyboard_trigger_key_up(ch - L'A' + KINC_KEY_A);
			}
			else if (ch >= L'0' && ch <= L'9') {
				kinc_internal_keyboard_trigger_key_up(ch - L'0' + KINC_KEY_0);
			}
			break;
		}
	}
}

static int getMouseX(NSEvent *event) {
	// TODO (DK) map [theEvent window] to window id instead of 0
	NSWindow *window = [[NSApplication sharedApplication] mainWindow];
	float scale = [window backingScaleFactor];
	return (int)([event locationInWindow].x * scale);
}

static int getMouseY(NSEvent *event) {
	// TODO (DK) map [theEvent window] to window id instead of 0
	NSWindow *window = [[NSApplication sharedApplication] mainWindow];
	float scale = [window backingScaleFactor];
	return (int)(kinc_height() - [event locationInWindow].y * scale);
}

static bool controlKeyMouseButton = false;

- (void)mouseDown:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		controlKeyMouseButton = true;
		kinc_internal_mouse_trigger_press(0, 1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		controlKeyMouseButton = false;
		kinc_internal_mouse_trigger_press(0, 0, getMouseX(theEvent), getMouseY(theEvent));
	}

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		kinc_internal_pen_trigger_press(0, getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)mouseUp:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	if (controlKeyMouseButton) {
		kinc_internal_mouse_trigger_release(0, 1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		kinc_internal_mouse_trigger_release(0, 0, getMouseX(theEvent), getMouseY(theEvent));
	}
	controlKeyMouseButton = false;

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		kinc_internal_pen_trigger_release(0, getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)mouseMoved:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	kinc_internal_mouse_trigger_move(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)mouseDragged:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	kinc_internal_mouse_trigger_move(0, getMouseX(theEvent), getMouseY(theEvent));

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		kinc_internal_pen_trigger_move(0, getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)rightMouseDown:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	kinc_internal_mouse_trigger_press(0, 1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseUp:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	kinc_internal_mouse_trigger_release(0, 1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	kinc_internal_mouse_trigger_move(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseDown:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_press(0, 2, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseUp:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_release(0, 2, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_move(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)scrollWheel:(NSEvent *)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	int delta = [theEvent deltaY];
	kinc_internal_mouse_trigger_scroll(0, -delta);
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
	NSPasteboard *pboard = [sender draggingPasteboard];
	NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	if ([[pboard types] containsObject:NSURLPboardType]) {
		if (sourceDragMask & NSDragOperationLink) {
			return NSDragOperationLink;
		}
	}
	return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
	NSPasteboard *pboard = [sender draggingPasteboard];
	// NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	if ([[pboard types] containsObject:NSURLPboardType]) {
		NSURL *fileURL = [NSURL URLFromPasteboard:pboard];
		wchar_t *filePath = (wchar_t *)[fileURL.path cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
		kinc_internal_drop_files_callback(filePath);
	}
	return YES;
}

#ifndef KINC_METAL
- (void)prepareOpenGL {
	const GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	[super prepareOpenGL];
}
#endif

- (void)update { // window resizes, moves and display changes (resize, depth and display config change)
#ifdef KINC_OPENGL
	[super update];
#endif
}

#ifndef KINC_METAL
- (id)initWithFrame:(NSRect)frameRect {
	NSOpenGLPixelFormat *pf = [BasicOpenGLView basicPixelFormat];
	self = [super initWithFrame:frameRect pixelFormat:pf];

	[self prepareOpenGL];
	[[self openGLContext] makeCurrentContext];
	[self setWantsBestResolutionOpenGLSurface:YES];
	return self;
}
#else

- (id)initWithFrame:(NSRect)frameRect {
	self = [super initWithFrame:frameRect];

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

	return self;
}
#endif

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)becomeFirstResponder {
	return YES;
}

- (BOOL)resignFirstResponder {
	return YES;
}

- (void)resize:(NSSize)size {
	[self setFrameSize:size];
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

void kinc_copy_to_clipboard(const char *text) {
	NSPasteboard *board = [NSPasteboard generalPasteboard];
	[board clearContents];
	[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
}
