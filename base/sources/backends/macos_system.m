#import "macos_system.h"
#import <Cocoa/Cocoa.h>
#include <stdbool.h>
#include <iron_system.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <iron_video.h>
#include <objc/runtime.h>
#include <mach/mach_time.h>

struct WindowData {
	id handle;
	id view;
	bool fullscreen;
	void (*resizeCallback)(int width, int height, void *data);
	void *resizeCallbackData;
	bool (*closeCallback)(void *data);
	void *closeCallbackData;
};

static struct WindowData windows[1] = {0};
static bool controlKeyMouseButton = false;
static int mouseX, mouseY;
static bool keyboardShown = false;
static const char *videoFormats[] = {"ogv", NULL};
static NSApplication *myapp;
static NSWindow *window;
static BasicMTKView *view;
static char language[3];
#ifdef WITH_GAMEPAD
static struct HIDManager *hidManager;
#endif

@implementation BasicMTKView

static bool shift = false;
static bool ctrl = false;
static bool alt = false;
static bool cmd = false;

- (void)flagsChanged:(NSEvent *)theEvent {
	if (shift) {
		iron_internal_keyboard_trigger_key_up(IRON_KEY_SHIFT);
		shift = false;
	}
	if (ctrl) {
		iron_internal_keyboard_trigger_key_up(IRON_KEY_CONTROL);
		ctrl = false;
	}
	if (alt) {
		iron_internal_keyboard_trigger_key_up(IRON_KEY_ALT);
		alt = false;
	}
	if (cmd) {
		iron_internal_keyboard_trigger_key_up(IRON_KEY_META);
		cmd = false;
	}

	if ([theEvent modifierFlags] & NSShiftKeyMask) {
		iron_internal_keyboard_trigger_key_down(IRON_KEY_SHIFT);
		shift = true;
	}
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		iron_internal_keyboard_trigger_key_down(IRON_KEY_CONTROL);
		ctrl = true;
	}
	if ([theEvent modifierFlags] & NSAlternateKeyMask) {
		iron_internal_keyboard_trigger_key_down(IRON_KEY_ALT);
		alt = true;
	}
	if ([theEvent modifierFlags] & NSCommandKeyMask) {
		iron_internal_keyboard_trigger_key_down(IRON_KEY_META);
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
			iron_internal_keyboard_trigger_key_down(IRON_KEY_SEMICOLON);
			break;
		case 91:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_OPEN_BRACKET);
			break;
		case 93:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_CLOSE_BRACKET);
			break;
		case 39:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_QUOTE);
			break;
		case 92:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_BACK_SLASH);
			break;
		case 44:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_COMMA);
			break;
		case 46:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_PERIOD);
			break;
		case 47:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_SLASH);
			break;
		case 96:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_BACK_QUOTE);
			break;
		case 32:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_SPACE);
			break;
		case 34:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_DOUBLE_QUOTE);
			break;
		case 40:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_OPEN_PAREN);
			break;
		case 41:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_CLOSE_PAREN);
			break;
		case 42:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_ASTERISK);
			break;
		case 43:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_PLUS);
			break;
		case 45:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_HYPHEN_MINUS);
			break;
		case 61:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_EQUALS);
			break;
		case 95:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_UNDERSCORE);
			break;
		}

		switch (ch) {
		case NSRightArrowFunctionKey:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_RIGHT);
			break;
		case NSLeftArrowFunctionKey:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_LEFT);
			break;
		case NSUpArrowFunctionKey:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_UP);
			break;
		case NSDownArrowFunctionKey:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_DOWN);
			break;
		case 27:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_ESCAPE);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_RETURN);
			iron_internal_keyboard_trigger_key_press('\n');
			break;
		case 0x7f:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_BACKSPACE);
			iron_internal_keyboard_trigger_key_press('\x08');
			break;
		case 9:
			iron_internal_keyboard_trigger_key_down(IRON_KEY_TAB);
			iron_internal_keyboard_trigger_key_press('\t');
			break;
		default:
			if (ch == 'x' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char *text = iron_internal_cut_callback();
				if (text != NULL) {
					NSPasteboard *board = [NSPasteboard generalPasteboard];
					[board clearContents];
					[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
				}
			}
			if (ch == 'c' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char *text = iron_internal_copy_callback();
				if (text != NULL) {
					iron_copy_to_clipboard(text);
				}
			}
			if (ch == 'v' && [theEvent modifierFlags] & NSCommandKeyMask) {
				NSPasteboard *board = [NSPasteboard generalPasteboard];
				NSString *data = [board stringForType:NSStringPboardType];
				if (data != nil) {
					char charData[4096];
					strcpy(charData, [data UTF8String]);
					iron_internal_paste_callback(charData);
				}
			}
			if (ch >= L'a' && ch <= L'z') {
				iron_internal_keyboard_trigger_key_down(ch - L'a' + IRON_KEY_A);
			}
			else if (ch >= L'A' && ch <= L'Z') {
				iron_internal_keyboard_trigger_key_down(ch - L'A' + IRON_KEY_A);
			}
			else if (ch >= L'0' && ch <= L'9') {
				iron_internal_keyboard_trigger_key_down(ch - L'0' + IRON_KEY_0);
			}
			iron_internal_keyboard_trigger_key_press(ch);
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
			iron_internal_keyboard_trigger_key_up(IRON_KEY_SEMICOLON);
			break;
		case 91:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_OPEN_BRACKET);
			break;
		case 93:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_CLOSE_BRACKET);
			break;
		case 39:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_QUOTE);
			break;
		case 92:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_BACK_SLASH);
			break;
		case 44:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_COMMA);
			break;
		case 46:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_PERIOD);
			break;
		case 47:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_SLASH);
			break;
		case 96:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_BACK_QUOTE);
			break;
		case 45:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_HYPHEN_MINUS);
			break;
		case 61:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_EQUALS);
			break;
		case NSRightArrowFunctionKey:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_RIGHT);
			break;
		case NSLeftArrowFunctionKey:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_LEFT);
			break;
		case NSUpArrowFunctionKey:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_UP);
			break;
		case NSDownArrowFunctionKey:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_DOWN);
			break;
		case 27:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_ESCAPE);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_RETURN);
			break;
		case 0x7f:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_BACKSPACE);
			break;
		case 9:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_TAB);
			break;
		case 32:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_SPACE);
			break;
		case 34:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_DOUBLE_QUOTE);
			break;
		case 40:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_OPEN_PAREN);
			break;
		case 41:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_CLOSE_PAREN);
			break;
		case 42:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_ASTERISK);
			break;
		case 43:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_PLUS);
			break;
		case 95:
			iron_internal_keyboard_trigger_key_up(IRON_KEY_UNDERSCORE);
			break;
		default:
			if (ch >= L'a' && ch <= L'z') {
				iron_internal_keyboard_trigger_key_up(ch - L'a' + IRON_KEY_A);
			}
			else if (ch >= L'A' && ch <= L'Z') {
				iron_internal_keyboard_trigger_key_up(ch - L'A' + IRON_KEY_A);
			}
			else if (ch >= L'0' && ch <= L'9') {
				iron_internal_keyboard_trigger_key_up(ch - L'0' + IRON_KEY_0);
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
	return (int)(iron_window_height() - [event locationInWindow].y * scale);
}

- (void)mouseDown:(NSEvent *)theEvent {
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		controlKeyMouseButton = true;
		iron_internal_mouse_trigger_press(1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		controlKeyMouseButton = false;
		iron_internal_mouse_trigger_press(0, getMouseX(theEvent), getMouseY(theEvent));
	}

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		iron_internal_pen_trigger_press(getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)mouseUp:(NSEvent *)theEvent {
	if (controlKeyMouseButton) {
		iron_internal_mouse_trigger_release(1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		iron_internal_mouse_trigger_release(0, getMouseX(theEvent), getMouseY(theEvent));
	}
	controlKeyMouseButton = false;

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		iron_internal_pen_trigger_release(getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)mouseMoved:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));
}

- (void)mouseDragged:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));

	if ([theEvent subtype] == NSTabletPointEventSubtype) {
		iron_internal_pen_trigger_move(getMouseX(theEvent), getMouseY(theEvent), theEvent.pressure);
	}
}

- (void)rightMouseDown:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_press(1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseUp:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_release(1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseDown:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_press(2, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseUp:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_release(2, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
	iron_internal_mouse_trigger_move(getMouseX(theEvent), getMouseY(theEvent));
}

- (void)scrollWheel:(NSEvent *)theEvent {
	int delta = [theEvent deltaY];
	iron_internal_mouse_trigger_scroll(-delta);
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
	if ([[pboard types] containsObject:NSURLPboardType]) {
		NSArray *urls = [pboard readObjectsForClasses:@[[NSURL class]] options:nil];
		for (NSURL *fileURL in urls) {
			wchar_t *filePath = (wchar_t *)[fileURL.path cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
			iron_internal_drop_files_callback(filePath);
		}
	}
	return YES;
}

- (void)update {
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

- (id<MTLCommandQueue>)metalQueue {
	return commandQueue;
}

@end

void iron_copy_to_clipboard(const char *text) {
	NSPasteboard *board = [NSPasteboard generalPasteboard];
	[board clearContents];
	[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
}

#ifdef WITH_GAMEPAD

static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);
static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender);
static void reset(struct HIDGamepad *gamepad);
static void initDeviceElements(struct HIDGamepad *gamepad, CFArrayRef elements);
static void buttonChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int buttonIndex);
static void axisChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex);

static void cstringFromCFStringRef(CFStringRef string, char *cstr, size_t clen) {
	cstr[0] = '\0';
	if (string != NULL) {
		char temp[256];
		if (CFStringGetCString(string, temp, 256, kCFStringEncodingUTF8)) {
			temp[iron_mini(255, (int)(clen - 1))] = '\0';
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
	gamepad->hidDeviceRef = inDeviceRef;
	gamepad->padIndex = inPadIndex;

	IOHIDDeviceOpen(gamepad->hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);
	IOHIDDeviceRegisterInputValueCallback(gamepad->hidDeviceRef, inputValueCallback, gamepad);
	IOHIDDeviceScheduleWithRunLoop(gamepad->hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	gamepad->hidQueueRef = IOHIDQueueCreate(kCFAllocatorDefault, gamepad->hidDeviceRef, 32, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(gamepad->hidQueueRef) == IOHIDQueueGetTypeID()) {
		IOHIDQueueStart(gamepad->hidQueueRef);
		IOHIDQueueRegisterValueAvailableCallback(gamepad->hidQueueRef, valueAvailableCallback, gamepad);
		IOHIDQueueScheduleWithRunLoop(gamepad->hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}

	CFArrayRef elementCFArrayRef = IOHIDDeviceCopyMatchingElements(gamepad->hidDeviceRef, NULL, kIOHIDOptionsTypeNone);
	initDeviceElements(gamepad, elementCFArrayRef);

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
}

static void initDeviceElements(struct HIDGamepad *gamepad, CFArrayRef elements) {

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
	if (gamepad->hidQueueRef) {
		IOHIDQueueStop(gamepad->hidQueueRef);
		IOHIDQueueUnscheduleFromRunLoop(gamepad->hidQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	}
	if (gamepad->hidDeviceRef) {
		IOHIDDeviceUnscheduleFromRunLoop(gamepad->hidDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDDeviceClose(gamepad->hidDeviceRef, kIOHIDOptionsTypeSeizeDevice);
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
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
	double min = IOHIDElementGetLogicalMin(elementRef);
	double max = IOHIDElementGetLogicalMax(elementRef);
	double normalize = (rawValue - min) / (max - min);
	iron_internal_gamepad_trigger_button(gamepad->padIndex, buttonIndex, normalize);
}

static void axisChanged(struct HIDGamepad *gamepad, IOHIDElementRef elementRef, IOHIDValueRef valueRef, int axisIndex) {
	double rawValue = IOHIDValueGetScaledValue(valueRef, kIOHIDValueScaleTypePhysical);
	double min = IOHIDElementGetPhysicalMin(elementRef);
	double max = IOHIDElementGetPhysicalMax(elementRef);
	double normalize = normalize = (((rawValue - min) / (max - min)) * 2) - 1;
	if (axisIndex % 2 == 1) {
		normalize = -normalize;
	}
	iron_internal_gamepad_trigger_axis(gamepad->padIndex, axisIndex, normalize);
}

static void inputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef) {}

static void valueAvailableCallback(void *inContext, IOReturn inResult, void *inSender) {
	struct HIDGamepad *pad = (struct HIDGamepad *)inContext;
	do {
		IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout((IOHIDQueueRef)inSender, 0.);
		if (!valueRef) {
			break;
		}

		IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);
		IOHIDElementCookie cookie = IOHIDElementGetCookie(elementRef);

		for (int i = 0, c = sizeof(pad->buttons); i < c; ++i) {
			if (cookie == pad->buttons[i]) {
				buttonChanged(pad, elementRef, valueRef, i);
				break;
			}
		}

		for (int i = 0, c = sizeof(pad->axis); i < c; ++i) {
			if (cookie == pad->axis[i]) {
				axisChanged(pad, elementRef, valueRef, i);
				break;
			}
		}

		CFRelease(valueRef);
	} while (1);
}

const char *iron_gamepad_vendor(int gamepad) {
	return "unknown";
}

const char *iron_gamepad_product_name(int gamepad) {
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
	manager->managerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (CFGetTypeID(manager->managerRef) == IOHIDManagerGetTypeID()) {

		CFMutableArrayRef matchingCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		if (matchingCFArrayRef) {
			CFDictionaryRef matchingCFDictRef = createDeviceMatchingDictionary(manager, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
			addMatchingArray(manager, matchingCFArrayRef, matchingCFDictRef);
			matchingCFDictRef = createDeviceMatchingDictionary(manager, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
			addMatchingArray(manager, matchingCFArrayRef, matchingCFDictRef);
		}
		else {
			iron_error("%s: CFArrayCreateMutable failed.", __PRETTY_FUNCTION__);
			return -1;
		}

		IOHIDManagerSetDeviceMatchingMultiple(manager->managerRef, matchingCFArrayRef);
		CFRelease(matchingCFArrayRef);
		IOHIDManagerOpen(manager->managerRef, kIOHIDOptionsTypeNone);
		IOHIDManagerRegisterDeviceMatchingCallback(manager->managerRef, deviceConnected, manager);
		IOHIDManagerRegisterDeviceRemovalCallback(manager->managerRef, deviceRemoved, manager);
		IOHIDManagerScheduleWithRunLoop(manager->managerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		return 0;
	}
	return -1;
}

bool addMatchingArray(struct HIDManager *manager, CFMutableArrayRef matchingCFArrayRef, CFDictionaryRef matchingCFDictRef) {
	if (matchingCFDictRef) {
		CFArrayAppendValue(matchingCFArrayRef, matchingCFDictRef);
		CFRelease(matchingCFDictRef);
		return true;
	}
	return false;
}

CFMutableDictionaryRef createDeviceMatchingDictionary(struct HIDManager *manager, uint32_t inUsagePage, uint32_t inUsage) {
	CFMutableDictionaryRef result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (result) {
		if (inUsagePage) {
			CFNumberRef pageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
			if (pageCFNumberRef) {
				CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsagePageKey), pageCFNumberRef);
				CFRelease(pageCFNumberRef);
				if (inUsage) {
					CFNumberRef usageCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
					if (usageCFNumberRef) {
						CFDictionarySetValue(result, CFSTR(kIOHIDDeviceUsageKey), usageCFNumberRef);
						CFRelease(usageCFNumberRef);
					}
				}
			}
		}
	}
	return result;
}

void deviceConnected(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	struct HIDManager *manager = (struct HIDManager *)inContext;
	struct HIDManagerDeviceRecord *device = &manager->devices[0];
	for (int i = 0; i < IRON_MAX_HID_DEVICES; ++i, ++device) {
		if (!device->connected) {
			device->connected = true;
			device->device = inIOHIDDeviceRef;
			HIDGamepad_bind(&device->pad, inIOHIDDeviceRef, i);
			break;
		}
	}
}

void deviceRemoved(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef) {
	struct HIDManager *manager = (struct HIDManager *)inContext;
	struct HIDManagerDeviceRecord *device = &manager->devices[0];
	for (int i = 0; i < IRON_MAX_HID_DEVICES; ++i, ++device) {
		if (device->connected && device->device == inIOHIDDeviceRef) {
			device->connected = false;
			device->device = NULL;
			HIDGamepad_unbind(&device->pad);
			break;
		}
	}
}

#endif

int iron_count_displays(void) {
	NSArray *screens = [NSScreen screens];
	return (int)[screens count];
}

int iron_primary_display(void) {
	NSArray *screens = [NSScreen screens];
	NSScreen *mainScreen = [NSScreen mainScreen];
	int max_displays = 8;
	for (int i = 0; i < max_displays; ++i) {
		if (mainScreen == screens[i]) {
			return i;
		}
	}
	return -1;
}

void iron_display_init(void) {}

iron_display_mode_t iron_display_current_mode(int display) {
	NSArray *screens = [NSScreen screens];
	NSScreen *screen = screens[display];
	NSRect screenRect = [screen frame];
	iron_display_mode_t dm;
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

NSWindow *iron_get_mac_window_handle();

void iron_internal_mouse_lock() {
	iron_mouse_hide();
}

void iron_internal_mouse_unlock(void) {
	iron_mouse_show();
}

bool iron_mouse_can_lock(void) {
	return true;
}

void iron_mouse_show(void) {
	CGDisplayShowCursor(kCGDirectMainDisplay);
}

void iron_mouse_hide(void) {
	CGDisplayHideCursor(kCGDirectMainDisplay);
}

void iron_mouse_set_position(int x, int y) {
	NSWindow *window = iron_get_mac_window_handle();
	float scale = [window backingScaleFactor];
	NSRect rect = [[NSScreen mainScreen] frame];

	CGPoint point;
	point.x = window.frame.origin.x + (x / scale);
	point.y = rect.size.height - (window.frame.origin.y + (y / scale));

	CGDisplayMoveCursorToPoint(0, point);
	CGAssociateMouseAndMouseCursorPosition(true);
}

void iron_mouse_get_position(int *x, int *y) {
	NSWindow *window = iron_get_mac_window_handle();
	NSPoint point = [window mouseLocationOutsideOfEventStream];
	*x = (int)point.x;
	*y = (int)point.y;
}

void iron_mouse_set_cursor(int cursor_index) {}

void iron_keyboard_show(void) {
	keyboardShown = true;
}

void iron_keyboard_hide(void) {
	keyboardShown = false;
}

bool iron_keyboard_active(void) {
	return keyboardShown;
}

const char *iron_system_id(void) {
	return "macOS";
}

const char **iron_video_formats(void) {
	return videoFormats;
}

void iron_set_keep_screen_on(bool on) {}

double iron_frequency(void) {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

uint64_t iron_timestamp(void) {
	return mach_absolute_time();
}

#ifdef WITH_GAMEPAD

bool iron_gamepad_connected(int num) {
	return true;
}

void iron_gamepad_rumble(int gamepad, float left, float right) {}

#endif

bool with_autoreleasepool(bool (*f)(void)) {
	@autoreleasepool {
		return f();
	}
}

const char *iron_get_resource_path(void) {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding];
}

@interface IronApplication : NSApplication {
}
- (void)terminate:(id)sender;
@end

@interface IronAppDelegate : NSObject <NSWindowDelegate> {
}
- (void)windowWillClose:(NSNotification *)notification;
- (void)windowDidResize:(NSNotification *)notification;
- (void)windowWillMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
- (void)windowDidResignMain:(NSNotification *)notification;
- (void)windowDidBecomeMain:(NSNotification *)notification;
@end

static IronAppDelegate *delegate;

CAMetalLayer *get_metal_layer(void) {
	return [view metalLayer];
}

id get_metal_device(void) {
	return [view metalDevice];
}

id get_metal_queue(void) {
	return [view metalQueue];
}

bool iron_internal_handle_messages(void) {
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

static void createWindow(iron_window_options_t *options) {
	int width = options->width / [[NSScreen mainScreen] backingScaleFactor];
	int height = options->height / [[NSScreen mainScreen] backingScaleFactor];
	int styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
	if ((options->features & IRON_WINDOW_FEATURE_RESIZEABLE) || (options->features & IRON_WINDOW_FEATURE_MAXIMIZABLE)) {
		styleMask |= NSWindowStyleMaskResizable;
	}
	if (options->features & IRON_WINDOW_FEATURE_MINIMIZABLE) {
		styleMask |= NSWindowStyleMaskMiniaturizable;
	}

	view = [[BasicMTKView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
	[view registerForDraggedTypes:[NSArray arrayWithObjects:NSURLPboardType, nil]];
	window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height) styleMask:styleMask backing:NSBackingStoreBuffered defer:TRUE];
	delegate = [IronAppDelegate alloc];
	[window setDelegate:delegate];
	[window setTitle:[NSString stringWithCString:options->title encoding:NSUTF8StringEncoding]];
	[window setAcceptsMouseMovedEvents:YES];
	[[window contentView] addSubview:view];
	[window center];

	windows[0].handle = window;
	windows[0].view = view;

	[window makeKeyAndOrderFront:nil];

	if (options->mode == IRON_WINDOW_MODE_FULLSCREEN) {
		[window toggleFullScreen:nil];
		windows[0].fullscreen = true;
	}
}

void iron_window_change_window_mode(iron_window_mode_t mode) {
	switch (mode) {
	case IRON_WINDOW_MODE_WINDOW:
		if (windows[0].fullscreen) {
			[window toggleFullScreen:nil];
			windows[0].fullscreen = false;
		}
		break;
	case IRON_WINDOW_MODE_FULLSCREEN:
		if (!windows[0].fullscreen) {
			[window toggleFullScreen:nil];
			windows[0].fullscreen = true;
		}
		break;
	}
}

void iron_window_set_close_callback(bool (*callback)(void *), void *data) {
	windows[0].closeCallback = callback;
	windows[0].closeCallbackData = data;
}

static void add_menubar(void) {
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

void iron_init(iron_window_options_t *win) {
	@autoreleasepool {
		myapp = [IronApplication sharedApplication];
		[myapp finishLaunching];
		[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
		NSApp.activationPolicy = NSApplicationActivationPolicyRegular;
		add_menubar();

		#ifdef WITH_GAMEPAD
		hidManager = (struct HIDManager *)malloc(sizeof(struct HIDManager));
		HIDManager_init(hidManager);
		#endif
	}

	createWindow(win);
	gpu_init(win->depth_bits, true);
}

int iron_window_width() {
	NSWindow *window = windows[0].handle;
	float scale = [window backingScaleFactor];
	return [[window contentView] frame].size.width * scale;
}

int iron_window_height() {
	NSWindow *window = windows[0].handle;
	float scale = [window backingScaleFactor];
	return [[window contentView] frame].size.height * scale;
}

NSWindow *iron_get_mac_window_handle() {
	return windows[0].handle;
}

void iron_load_url(const char *url) {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

const char *iron_language(void) {
	NSString *nsstr = [[NSLocale preferredLanguages] objectAtIndex:0];
	const char *lang = [nsstr UTF8String];
	language[0] = lang[0];
	language[1] = lang[1];
	language[2] = 0;
	return language;
}

void iron_internal_shutdown(void) {}

const char *iron_internal_save_path(void) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *resolvedPath = [paths objectAtIndex:0];
	NSString *appName = [NSString stringWithUTF8String:iron_application_name()];
	resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];

	NSFileManager *fileMgr = [[NSFileManager alloc] init];

	NSError *error;
	[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];

	resolvedPath = [resolvedPath stringByAppendingString:@"/"];
	return [resolvedPath cStringUsingEncoding:NSUTF8StringEncoding];
}

#ifndef IRON_NO_MAIN
int main(int argc, char **argv) {
	return kickstart(argc, argv);
}
#endif

@implementation IronApplication

- (void)terminate:(id)sender {
	iron_stop();
}

@end

@implementation IronAppDelegate
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
	iron_stop();
}

void iron_internal_call_resize_callback(int width, int height) {
	if (windows[0].resizeCallback != NULL) {
		windows[0].resizeCallback(width, height, windows[0].resizeCallbackData);
	}
}

- (void)windowDidResize:(NSNotification *)notification {
	NSWindow *window = [notification object];
	NSSize size = [[window contentView] frame].size;
	[view resize:size];

	float scale = [window backingScaleFactor];
	int w = size.width * scale;
	int h = size.height * scale;

	gpu_resize(w, h);
	iron_internal_call_resize_callback(w, h);
}

- (void)windowWillMiniaturize:(NSNotification *)notification {
	iron_internal_background_callback();
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
	iron_internal_foreground_callback();
}

- (void)windowDidResignMain:(NSNotification *)notification {
	iron_internal_pause_callback();
}

- (void)windowDidBecomeMain:(NSNotification *)notification {
	iron_internal_resume_callback();
}

@end

int iron_window_x() {
	return 0;
}

int iron_window_y() {
	return 0;
}

void iron_window_resize(int width, int height) {}
void iron_window_move(int x, int y) {}
void iron_window_change_mode(iron_window_mode_t mode) {}
void iron_window_destroy() {}
void iron_window_show() {}
void iron_window_hide() {}
void iron_window_set_title(const char *title) {}
void iron_window_create(iron_window_options_t *win) {}

void iron_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {
	windows[0].resizeCallback = callback;
	windows[0].resizeCallbackData = data;
}

iron_window_mode_t iron_window_get_mode() {
	return IRON_WINDOW_MODE_WINDOW;
}

int iron_window_display() {
	return 0;
}
