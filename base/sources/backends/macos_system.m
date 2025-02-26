#import "macos_system.h"
#include <stdbool.h>
#include <iron_system.h>
#include <iron_gpu.h>
#include <iron_math.h>
#import <Cocoa/Cocoa.h>
#include <objc/runtime.h>
#include <iron_video.h>

struct WindowData {
	id handle;
	id view;
	bool fullscreen;
	void (*resizeCallback)(int width, int height, void *data);
	void *resizeCallbackData;
	bool (*closeCallback)(void *data);
	void *closeCallbackData;
};

static struct WindowData windows[16] = {};

@implementation BasicOpenGLView

static bool shift = false;
static bool ctrl = false;
static bool alt = false;
static bool cmd = false;

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
	NSWindow *window = [[NSApplication sharedApplication] mainWindow];
	float scale = [window backingScaleFactor];
	return (int)([event locationInWindow].x * scale);
}

static int getMouseY(NSEvent *event) {
	NSWindow *window = [[NSApplication sharedApplication] mainWindow];
	float scale = [window backingScaleFactor];
	return (int)(kinc_height() - [event locationInWindow].y * scale);
}

static bool controlKeyMouseButton = false;

- (void)mouseDown:(NSEvent *)theEvent {
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		controlKeyMouseButton = true;
		kinc_internal_mouse_trigger_press(1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		controlKeyMouseButton = false;
		kinc_internal_mouse_trigger_press(0, getMouseX(theEvent), getMouseY(theEvent));
	}

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		kinc_internal_pen_trigger_press(getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)mouseUp:(NSEvent *)theEvent {
	if (controlKeyMouseButton) {
		kinc_internal_mouse_trigger_release(1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		kinc_internal_mouse_trigger_release(0, getMouseX(theEvent), getMouseY(theEvent));
	}
	controlKeyMouseButton = false;

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		kinc_internal_pen_trigger_release(getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)mouseMoved:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));
}

- (void)mouseDragged:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		kinc_internal_pen_trigger_move(getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)rightMouseDown:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_press(1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseUp:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_release(1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseDown:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_press(2, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseUp:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_release(2, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
	kinc_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));
}

- (void)scrollWheel:(NSEvent *)theEvent {
	int delta = [theEvent deltaY];
	kinc_internal_mouse_trigger_scroll(-delta);
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

- (void)update { // window resizes, moves and display changes (resize, depth and display config change)
}

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

@end

void kinc_copy_to_clipboard(const char *text) {
	NSPasteboard *board = [NSPasteboard generalPasteboard];
	[board clearContents];
	[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
}

static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);
static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender);

static void reset(struct HIDGamepad *gamepad);

static void initDeviceElements(struct HIDGamepad *gamepad, CFArrayRef elements);

static void buttonChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex);
static void axisChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex);

static bool debugButtonInput = false;

static void logButton(int buttonIndex, bool pressed) {
	switch (buttonIndex) {
	case 0:
		kinc_log(KINC_LOG_LEVEL_INFO, "A Pressed %i", pressed);
		break;

	case 1:
		kinc_log(KINC_LOG_LEVEL_INFO, "B Pressed %i", pressed);
		break;

	case 2:
		kinc_log(KINC_LOG_LEVEL_INFO, "X Pressed %i", pressed);
		break;

	case 3:
		kinc_log(KINC_LOG_LEVEL_INFO, "Y Pressed %i", pressed);
		break;

	case 4:
		kinc_log(KINC_LOG_LEVEL_INFO, "Lb Pressed %i", pressed);
		break;

	case 5:
		kinc_log(KINC_LOG_LEVEL_INFO, "Rb Pressed %i", pressed);
		break;

	case 6:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left Stick Pressed %i", pressed);
		break;

	case 7:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right Stick Pressed %i", pressed);
		break;

	case 8:
		kinc_log(KINC_LOG_LEVEL_INFO, "Start Pressed %i", pressed);
		break;

	case 9:
		kinc_log(KINC_LOG_LEVEL_INFO, "Back Pressed %i", pressed);
		break;

	case 10:
		kinc_log(KINC_LOG_LEVEL_INFO, "Home Pressed %i", pressed);
		break;

	case 11:
		kinc_log(KINC_LOG_LEVEL_INFO, "Up Pressed %i", pressed);
		break;

	case 12:
		kinc_log(KINC_LOG_LEVEL_INFO, "Down Pressed %i", pressed);
		break;

	case 13:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left Pressed %i", pressed);
		break;

	case 14:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right Pressed %i", pressed);
		break;

	default:
		break;
	}
}

static bool debugAxisInput = false;

static void logAxis(int axisIndex) {
	switch (axisIndex) {
	case 0:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left stick X");
		break;

	case 1:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left stick Y");
		break;

	case 2:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right stick X");
		break;

	case 3:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right stick Y");
		break;

	case 4:
		kinc_log(KINC_LOG_LEVEL_INFO, "Left trigger");
		break;

	case 5:
		kinc_log(KINC_LOG_LEVEL_INFO, "Right trigger");
		break;

	default:
		break;
	}
}

// Helper function to copy a CFStringRef to a cstring buffer.
// CFStringRef is converted to UTF8 and as many characters as possible are
// placed into the buffer followed by a null terminator.
// The buffer is set to an empty string if the conversion fails.
static void cstringFromCFStringRef(CFStringRef string, char *cstr, size_t clen) {
	cstr[0] = '\0';
	if (string != NULL) {
		char temp[256];
		if (CFStringGetCString(string, temp, 256, kCFStringEncodingUTF8)) {
			temp[kinc_mini(255, (int)(clen - 1))] = '\0';
			strncpy(cstr, temp, clen);
		}
	}
}

void HIDGamepad_init(struct HIDGamepad *gamepad) {
	reset(gamepad);
}

void HIDGamepad_destroy(struct HIDGamepad *gamepad) {
	HIDGamepad_unbind(gamepad);
}

void HIDGamepad_bind(struct HIDGamepad *gamepad, IOHIDDeviceRef inDeviceRef, int inPadIndex) {
	kinc_affirm(inDeviceRef != NULL);
	kinc_affirm(inPadIndex >= 0);
	kinc_affirm(gamepad->hidDeviceRef == NULL);
	kinc_affirm(gamepad->hidQueueRef == NULL);
	kinc_affirm(gamepad->padIndex == -1);

	// Set device and device index
	gamepad->hidDeviceRef = inDeviceRef;
	gamepad->padIndex = inPadIndex;

	// Initialise HID Device
	// ...open device
	IOHIDDeviceOpen(gamepad->hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);

	// ..register callbacks
	IOHIDDeviceRegisterInputValueCallback(gamepad->hidDeviceRef, inputValueCallback, gamepad);
	IOHIDDeviceScheduleWithRunLoop(gamepad->hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	// ...create a queue to access element values
	gamepad->hidQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, gamepad->hidDeviceRef, 32, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(gamepad->hidQueueRef) == IOHIDQueueGetTypeID()) {
		IOHIDQueueStart(gamepad->hidQueueRef);
		IOHIDQueueRegisterValueAvailableCallback(gamepad->hidQueueRef, valueAvailableCallback, gamepad);
		IOHIDQueueScheduleWithRunLoop(gamepad->hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	// ...get all elements (buttons, axes)
	CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(gamepad->hidDeviceRef, NULL, kIOHIDOptionsTypeNone);
	initDeviceElements(gamepad, elementCFArrayRef);

	// ...get device manufacturer and product details
	{
		CFNumberRef vendorIdRef = (CFNumberRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDVendorIDKey));
		CFNumberGetValue(vendorIdRef, kCFNumberIntType, &gamepad->hidDeviceVendorID);

		CFNumberRef productIdRef = (CFNumberRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDProductIDKey));
		CFNumberGetValue(productIdRef, kCFNumberIntType, &gamepad->hidDeviceProductID);

		CFStringRef vendorRef = (CFStringRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDManufacturerKey));
		cstringFromCFStringRef(vendorRef, gamepad->hidDeviceVendor, sizeof(gamepad->hidDeviceVendor));

		CFStringRef productRef = (CFStringRef)IOHIDDeviceGetProperty(gamepad->hidDeviceRef, CFSTR(kIOHIDProductKey));
		cstringFromCFStringRef(productRef, gamepad->hidDeviceProduct, sizeof(gamepad->hidDeviceProduct));
	}

	kinc_log(KINC_LOG_LEVEL_INFO, "HIDGamepad.bind: <%p> idx:%d [0x%x:0x%x] [%s] [%s]", inDeviceRef, gamepad->padIndex, gamepad->hidDeviceVendorID,
	         gamepad->hidDeviceProductID, gamepad->hidDeviceVendor, gamepad->hidDeviceProduct);
}

static void initDeviceElements(struct HIDGamepad *gamepad, CFArrayRef elements) {
	kinc_affirm(elements != NULL);

	for (CFIndex i = 0, count = CFArrayGetCount(elements); i < count; ++i) {
		IOHIDElementRef elementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		IOHIDElementType elemType = IOHIDElementGetType(elementRef);

		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);

		uint32_t usagePage = IOHIDElementGetUsagePage(elementRef);
		uint32_t usage = IOHIDElementGetUsage(elementRef);

		// Match up items
		switch (usagePage) {
		case kHIDPage_GenericDesktop:
			switch (usage) {
			case kHIDUsage_GD_X: // Left stick X
				gamepad->axis[0] = cookie;
				break;
			case kHIDUsage_GD_Y: // Left stick Y
				gamepad->axis[1] = cookie;
				break;
			case kHIDUsage_GD_Z: // Left trigger
				gamepad->axis[4] = cookie;
				break;
			case kHIDUsage_GD_Rx: // Right stick X
				gamepad->axis[2] = cookie;
				break;
			case kHIDUsage_GD_Ry: // Right stick Y
				gamepad->axis[3] = cookie;
				break;
			case kHIDUsage_GD_Rz: // Right trigger
				gamepad->axis[5] = cookie;
				break;
			case kHIDUsage_GD_Hatswitch:
				break;
			default:
				break;
			}
			break;
		case kHIDPage_Button:
			if ((usage >= 1) && (usage <= 15)) {
				// Button 1-11
				gamepad->buttons[usage - 1] = cookie;
			}
			break;
		default:
			break;
		}

		if (elemType == kIOHIDElementTypeInput_Misc || elemType == kIOHIDElementTypeInput_Button || elemType == kIOHIDElementTypeInput_Axis) {
			if (!IOHIDQueueContainsElement(gamepad->hidQueueRef, elementRef))
				IOHIDQueueAddElement(gamepad->hidQueueRef, elementRef);
		}
	}
}

void HIDGamepad_unbind(struct HIDGamepad *gamepad) {
	kinc_log(KINC_LOG_LEVEL_INFO, "HIDGamepad.unbind: idx:%d [0x%x:0x%x] [%s] [%s]", gamepad->padIndex, gamepad->hidDeviceVendorID, gamepad->hidDeviceProductID,
	         gamepad->hidDeviceVendor, gamepad->hidDeviceProduct);

	if (gamepad->hidQueueRef) {
		IOHIDQueueStop(gamepad->hidQueueRef);
		IOHIDQueueUnscheduleFromRunLoop(gamepad->hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	if (gamepad->hidDeviceRef) {
		IOHIDDeviceUnscheduleFromRunLoop(gamepad->hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDDeviceClose(gamepad->hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);
	}

	if (gamepad->padIndex >= 0) {
	}

	reset(gamepad);
}

static void reset(struct HIDGamepad *gamepad) {
	gamepad->padIndex = -1;
	gamepad->hidDeviceRef = NULL;
	gamepad->hidQueueRef = NULL;
	gamepad->hidDeviceVendor[0] = '\0';
	gamepad->hidDeviceProduct[0] = '\0';
	gamepad->hidDeviceVendorID = 0;
	gamepad->hidDeviceProductID = 0;

	memset(gamepad->axis, 0, sizeof(gamepad->axis));
	memset(gamepad->buttons, 0, sizeof(gamepad->buttons));
}

static void buttonChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize button value to the range [0.0, 1.0] (0 - release, 1 - pressed)
	double min = IOHIDElementGetLogicalMin(elementRef);
	double max = IOHIDElementGetLogicalMax(elementRef);
	double normalize = (rawValue - min) / (max - min);

	kinc_internal_gamepad_trigger_button(gamepad->padIndex, buttonIndex, normalize);

	if (debugButtonInput)
		logButton(buttonIndex, (normalize != 0));
}

static void axisChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex) {
	// double rawValue = IOHIDValueGetIntegerValue(valueRef);
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);

	// Normalize axis value to the range [-1.0, 1.0] (e.g. -1 - left, 0 - release, 1 - right)
	double min = IOHIDElementGetPhysicalMin(elementRef);
	double max = IOHIDElementGetPhysicalMax(elementRef);
	double normalize = normalize = (((rawValue - min) / (max - min)) * 2) - 1;

	// Invert Y axis
	if (axisIndex % 2 == 1)
		normalize = -normalize;

	kinc_internal_gamepad_trigger_axis(gamepad->padIndex, axisIndex, normalize);

	if (debugAxisInput)
		logAxis(axisIndex);
}

static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef) {}

static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender) {
	struct HIDGamepad *pad = (struct HIDGamepad *)inContext;
	do {
		IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout((IOHIDQueueRef)inSender, 0.);
		if (!valueRef)
			break;

		// process the HID value reference
		IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);

		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);

		// Check button
		for (int i = 0, c = sizeof(pad->buttons); i < c; ++i) {
			if (cookie == pad->buttons[i]) {
				buttonChanged(pad, elementRef, valueRef, i);
				break;
			}
		}

		// Check axes
		for (int i = 0, c = sizeof(pad->axis); i < c; ++i) {
			if (cookie == pad->axis[i]) {
				axisChanged(pad, elementRef, valueRef, i);
				break;
			}
		}

		CFRelease(valueRef);
	} while (1);
}

const char *kinc_gamepad_vendor(int gamepad) {
	return "unknown";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "unknown";
}

static int initHIDManager(struct HIDManager *manager);
static bool addMatchingArray(struct HIDManager *manager, CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef);
static CFMutableDictionaryRef createDeviceMatchingDictionary(struct HIDManager *manager, uint32_t inUsagePage, uint32_t inUsage);

static void deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);
static void deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);

void HIDManager_init(struct HIDManager *manager) {
	manager->managerRef = 0x0;
	initHIDManager(manager);
}

void HIDManager_destroy(struct HIDManager *manager) {
	if (manager->managerRef) {
		IOHIDManagerUnscheduleFromRunLoop(manager->managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDManagerClose(manager->managerRef, kIOHIDOptionsTypeNone);
	}
}

static int initHIDManager(struct HIDManager *manager) {
	// Initialize the IOHIDManager
	manager->managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(manager->managerRef) == IOHIDManagerGetTypeID()) {

		// Create a matching dictionary for gamepads and joysticks
		CFMutableArrayRef matchingCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (matchingCFArrayRef) {
			// Create a device matching dictionary for joysticks
			CFDictionaryRef matchingCFDictRef = createDeviceMatchingDictionary(manager, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
			addMatchingArray(manager, matchingCFArrayRef, matchingCFDictRef);

			// Create a device matching dictionary for game pads
			matchingCFDictRef = createDeviceMatchingDictionary(manager, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
			addMatchingArray(manager, matchingCFArrayRef, matchingCFDictRef);
		}
		else {
			kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFArrayCreateMutable failed.", __PRETTY_FUNCTION__);
			return -1;
		}

		// Set the HID device matching array
		IOHIDManagerSetDeviceMatchingMultiple(manager->managerRef, matchingCFArrayRef);
		CFRelease(matchingCFArrayRef);

		// Open manager
		IOHIDManagerOpen(manager->managerRef, kIOHIDOptionsTypeNone);

		// Register routines to be called when (matching) devices are connected or disconnected
		IOHIDManagerRegisterDeviceMatchingCallback(manager->managerRef, deviceConnected, manager);
		IOHIDManagerRegisterDeviceRemovalCallback(manager->managerRef, deviceRemoved, manager);

		IOHIDManagerScheduleWithRunLoop(manager->managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

		return 0;
	}
	return -1;
}

bool addMatchingArray(struct HIDManager *manager, CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef) {
	if (matchingCFDictRef) {
		// Add it to the matching array
		CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
		CFRelease(matchingCFDictRef); // and release it
		return true;
	}
	return false;
}

CFMutableDictionaryRef createDeviceMatchingDictionary(struct HIDManager *manager, uint32_t inUsagePage, uint32_t inUsage) {
	// Create a dictionary to add usage page/usages to
	CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (result) {
		if (inUsagePage) {
			// Add key for device type to refine the matching dictionary.
			CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
			if (pageCFNumberRef) {
				CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
				CFRelease(pageCFNumberRef);

				// note: the usage is only valid if the usage page is also defined
				if (inUsage) {
					CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
					if (usageCFNumberRef) {
						CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
						CFRelease(usageCFNumberRef);
					}
					else {
						kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFNumberCreate(usage) failed.", __PRETTY_FUNCTION__);
					}
				}
			}
			else {
				kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFNumberCreate(usage page) failed.", __PRETTY_FUNCTION__);
			}
		}
	}
	else {
		kinc_log(KINC_LOG_LEVEL_ERROR, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__);
	}
	return result;
}

// HID device plugged callback
void deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	// Reference manager
	struct HIDManager *manager = (struct HIDManager *)inContext;

	// Find an empty slot in the devices list and add the new device there
	// TODO: does this need to be made thread safe?
	struct HIDManagerDeviceRecord *device = &manager->devices[0];
	for (int i = 0; i < KINC_MAX_HID_DEVICES; ++i, ++device) {
		if (!device->connected) {
			device->connected = true;
			device->device = inIOHIDDeviceRef;
			HIDGamepad_bind(&device->pad, inIOHIDDeviceRef, i);
			break;
		}
	}
}

// HID device unplugged callback
void deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	// Reference manager
	struct HIDManager *manager = (struct HIDManager *)inContext;

	// TODO: does this need to be made thread safe?
	struct HIDManagerDeviceRecord *device = &manager->devices[0];
	for (int i = 0; i < KINC_MAX_HID_DEVICES; ++i, ++device) {
		// TODO: is comparing IOHIDDeviceRef to match devices safe? Is there a better way?
		if (device->connected && device->device == inIOHIDDeviceRef) {
			device->connected = false;
			device->device = NULL;
			HIDGamepad_unbind(&device->pad);
			break;
		}
	}
}

#define maxDisplays 10

int kinc_count_displays(void) {
	NSArray *screens = [NSScreen screens];
	return (int)[screens count];
}

int kinc_primary_display(void) {
	NSArray *screens = [NSScreen screens];
	NSScreen *mainScreen = [NSScreen mainScreen];
	for (int i = 0; i < maxDisplays; ++i) {
		if (mainScreen == screens[i]) {
			return i;
		}
	}
	return -1;
}

void kinc_display_init(void) {}

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
	kinc_display_mode_t dm;
	dm.width = 800;
	dm.height = 600;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;
	return dm;
}

int kinc_display_count_available_modes(int display) {
	return 1;
}

bool kinc_display_available(int display) {
	return true;
}

const char *kinc_display_name(int display) {
	return "Display";
}

kinc_display_mode_t kinc_display_current_mode(int display) {
	NSArray *screens = [NSScreen screens];
	NSScreen *screen = screens[display];
	NSRect screenRect = [screen frame];
	kinc_display_mode_t dm;
	dm.width = screenRect.size.width;
	dm.height = screenRect.size.height;
	dm.frequency = 60;
	dm.bits_per_pixel = 32;

	NSDictionary *description = [screen deviceDescription];
	NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
	NSNumber *screenNumber = [description objectForKey:@"NSScreenNumber"];
	CGSize displayPhysicalSize = CGDisplayScreenSize([screenNumber unsignedIntValue]); // in millimeters
	double ppi = displayPixelSize.width / (displayPhysicalSize.width * 0.039370);      // Convert MM to INCH
	dm.pixels_per_inch = round(ppi);

	return dm;
}

NSWindow *kinc_get_mac_window_handle();

void kinc_internal_mouse_lock() {
	kinc_mouse_hide();
}

void kinc_internal_mouse_unlock(void) {
	kinc_mouse_show();
}

bool kinc_mouse_can_lock(void) {
	return true;
}

void kinc_mouse_show(void) {
	CGDisplayShowCursor(kCGDirectMainDisplay);
}

void kinc_mouse_hide(void) {
	CGDisplayHideCursor(kCGDirectMainDisplay);
}

void kinc_mouse_set_position(int x, int y) {

	NSWindow *window = kinc_get_mac_window_handle();
	float scale = [window backingScaleFactor];
	NSRect rect = [[NSScreen mainScreen] frame];

	CGPoint point;
	point.x = window.frame.origin.x + (x / scale);
	point.y = rect.size.height - (window.frame.origin.y + (y / scale));

	CGDisplayMoveCursorToPoint(0, point);
	CGAssociateMouseAndMouseCursorPosition(true);
}

void kinc_mouse_get_position(int *x, int *y) {
	NSWindow *window = kinc_get_mac_window_handle();
	NSPoint point = [window mouseLocationOutsideOfEventStream];
	*x = (int)point.x;
	*y = (int)point.y;
}

void kinc_mouse_set_cursor(int cursor_index) {}

static int mouseX, mouseY;
static bool keyboardShown = false;

void Kinc_Mouse_GetPosition(int *x, int *y) {
	*x = mouseX;
	*y = mouseY;
}

void kinc_keyboard_show(void) {
	keyboardShown = true;
}

void kinc_keyboard_hide(void) {
	keyboardShown = false;
}

bool kinc_keyboard_active(void) {
	return keyboardShown;
}

const char *kinc_system_id(void) {
	return "macOS";
}

static const char *videoFormats[] = {"ogv", NULL};

const char **kinc_video_formats(void) {
	return videoFormats;
}

void kinc_set_keep_screen_on(bool on) {}

#include <mach/mach_time.h>

double kinc_frequency(void) {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

kinc_ticks_t kinc_timestamp(void) {
	return mach_absolute_time();
}

bool kinc_gamepad_connected(int num) {
	return true;
}

void kinc_gamepad_rumble(int gamepad, float left, float right) {}

bool withAutoreleasepool(bool (*f)(void)) {
	@autoreleasepool {
		return f();
	}
}

extern const char *macgetresourcepath(void);

const char *macgetresourcepath(void) {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding];
}

@interface KincApplication : NSApplication {
}
- (void)terminate:(id)sender;
@end

@interface KincAppDelegate : NSObject <NSWindowDelegate> {
}
- (void)windowWillClose:(NSNotification *)notification;
- (void)windowDidResize:(NSNotification *)notification;
- (void)windowWillMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
- (void)windowDidResignMain:(NSNotification *)notification;
- (void)windowDidBecomeMain:(NSNotification *)notification;
@end

static NSApplication *myapp;
static NSWindow *window;
static BasicOpenGLView *view;
static KincAppDelegate *delegate;
static struct HIDManager *hidManager;

CAMetalLayer *getMetalLayer(void) {
	return [view metalLayer];
}

id getMetalDevice(void) {
	return [view metalDevice];
}

id getMetalLibrary(void) {
	return [view metalLibrary];
}

id getMetalQueue(void) {
	return [view metalQueue];
}

bool kinc_internal_handle_messages(void) {
	NSEvent *event = [myapp nextEventMatchingMask:NSAnyEventMask
	                                    untilDate:[NSDate distantPast]
	                                       inMode:NSDefaultRunLoopMode
	                                      dequeue:YES]; // distantPast: non-blocking
	if (event != nil) {
		[myapp sendEvent:event];
		[myapp updateWindows];
	}

	// Sleep for a frame to limit the calls when the window is not visible.
	if (!window.visible) {
		[NSThread sleepForTimeInterval:1.0 / 60];
	}
	return true;
}

void swapBuffersMac() {
}

static void createWindow(kinc_window_options_t *options) {
	int width = options->width / [[NSScreen mainScreen] backingScaleFactor];
	int height = options->height / [[NSScreen mainScreen] backingScaleFactor];
	int styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
	if ((options->window_features & KINC_WINDOW_FEATURE_RESIZEABLE) || (options->window_features & KINC_WINDOW_FEATURE_MAXIMIZABLE)) {
		styleMask |= NSWindowStyleMaskResizable;
	}
	if (options->window_features & KINC_WINDOW_FEATURE_MINIMIZABLE) {
		styleMask |= NSWindowStyleMaskMiniaturizable;
	}

	view = [[BasicOpenGLView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
	[view registerForDraggedTypes:[NSArray arrayWithObjects:NSURLPboardType, nil]];
	window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height) styleMask:styleMask backing:NSBackingStoreBuffered defer:TRUE];
	delegate = [KincAppDelegate alloc];
	[window setDelegate:delegate];
	[window setTitle:[NSString stringWithCString:options->title encoding:NSUTF8StringEncoding]];
	[window setAcceptsMouseMovedEvents:YES];
	[[window contentView] addSubview:view];
	[window center];

	windows[0].handle = window;
	windows[0].view = view;

	[window makeKeyAndOrderFront:nil];

	if (options->mode == KINC_WINDOW_MODE_FULLSCREEN) {
		[window toggleFullScreen:nil];
		windows[0].fullscreen = true;
	}
}

void kinc_window_change_window_mode(kinc_window_mode_t mode) {
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW:
		if (windows[0].fullscreen) {
			[window toggleFullScreen:nil];
			windows[0].fullscreen = false;
		}
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
		if (!windows[0].fullscreen) {
			[window toggleFullScreen:nil];
			windows[0].fullscreen = true;
		}
		break;
	}
}

void kinc_window_set_close_callback(bool (*callback)(void *), void *data) {
	windows[0].closeCallback = callback;
	windows[0].closeCallbackData = data;
}

static void addMenubar(void) {
	NSString *appName = [[NSProcessInfo processInfo] processName];

	NSMenu *appMenu = [NSMenu new];
	NSString *quitTitle = [@"Quit " stringByAppendingString:appName];
	NSMenuItem *quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
	[appMenu addItem:quitMenuItem];

	NSMenuItem *appMenuItem = [NSMenuItem new];
	[appMenuItem setSubmenu:appMenu];

	NSMenu *menubar = [NSMenu new];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];
}

void kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	@autoreleasepool {
		myapp = [KincApplication sharedApplication];
		[myapp finishLaunching];
		[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
		NSApp.activationPolicy = NSApplicationActivationPolicyRegular;

		hidManager = (struct HIDManager *)malloc(sizeof(struct HIDManager));
		HIDManager_init(hidManager);
		addMenubar();
	}

	kinc_window_options_t defaultWindowOptions;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWindowOptions);
		win = &defaultWindowOptions;
	}

	kinc_framebuffer_options_t defaultFramebufferOptions;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFramebufferOptions);
		frame = &defaultFramebufferOptions;
	}

	win->width = width;
	win->height = height;
	if (win->title == NULL) {
		win->title = name;
	}

	createWindow(win);
	kinc_g5_internal_init();
	kinc_g4_internal_init_window(frame->depth_bits, true);
}

int kinc_window_width() {
	NSWindow *window = windows[0].handle;
	float scale = [window backingScaleFactor];
	return [[window contentView] frame].size.width * scale;
}

int kinc_window_height() {
	NSWindow *window = windows[0].handle;
	float scale = [window backingScaleFactor];
	return [[window contentView] frame].size.height * scale;
}

NSWindow *kinc_get_mac_window_handle() {
	return windows[0].handle;
}

void kinc_load_url(const char *url) {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

static char language[3];

const char *kinc_language(void) {
	NSString *nsstr = [[NSLocale preferredLanguages] objectAtIndex:0];
	const char *lang = [nsstr UTF8String];
	language[0] = lang[0];
	language[1] = lang[1];
	language[2] = 0;
	return language;
}

void kinc_internal_shutdown(void) {}

static const char *getSavePath(void) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *resolvedPath = [paths objectAtIndex:0];
	NSString *appName = [NSString stringWithUTF8String:kinc_application_name()];
	resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];

	NSFileManager *fileMgr = [[NSFileManager alloc] init];

	NSError *error;
	[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];

	resolvedPath = [resolvedPath stringByAppendingString:@"/"];
	return [resolvedPath cStringUsingEncoding:NSUTF8StringEncoding];
}

const char *kinc_internal_save_path(void) {
	return getSavePath();
}

#ifndef KINC_NO_MAIN
int main(int argc, char **argv) {
	return kickstart(argc, argv);
}
#endif

@implementation KincApplication

- (void)terminate:(id)sender {
	kinc_stop();
}

@end

@implementation KincAppDelegate
- (BOOL)windowShouldClose:(NSWindow *)sender {
	if (windows[0].closeCallback != NULL) {
		if (windows[0].closeCallback(windows[0].closeCallbackData)) {
			return YES;
		}
		else {
			return NO;
		}
	}
	return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
	kinc_stop();
}

- (void)windowDidResize:(NSNotification *)notification {
	NSWindow *window = [notification object];
	NSSize size = [[window contentView] frame].size;
	[view resize:size];
	if (windows[0].resizeCallback != NULL) {
		windows[0].resizeCallback(size.width, size.height, windows[0].resizeCallbackData);
	}
}

- (void)windowWillMiniaturize:(NSNotification *)notification {
	kinc_internal_background_callback();
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
	kinc_internal_foreground_callback();
}

- (void)windowDidResignMain:(NSNotification *)notification {
	kinc_internal_pause_callback();
}

- (void)windowDidBecomeMain:(NSNotification *)notification {
	kinc_internal_resume_callback();
}

@end

int kinc_window_x() {
	return 0;
}

int kinc_window_y() {
	return 0;
}

void kinc_window_resize(int width, int height) {}

void kinc_window_move(int x, int y) {}

void kinc_window_change_features(int features) {}

void kinc_window_change_mode(kinc_window_mode_t mode) {}

void kinc_window_destroy() {}

void kinc_window_show() {}

void kinc_window_hide() {}

void kinc_window_set_title(const char *title) {}

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
}

void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {
	windows[0].resizeCallback = callback;
	windows[0].resizeCallbackData = data;
}

kinc_window_mode_t kinc_window_get_mode() {
	return KINC_WINDOW_MODE_WINDOW;
}

int kinc_window_display() {
	return 0;
}
