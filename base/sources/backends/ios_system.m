#include "ios_system.h"
#include <iron_gpu.h>
#include <iron_system.h>
#import <Foundation/Foundation.h>
#include <iron_math.h>
#include <objc/runtime.h>
#import <AVFAudio/AVFAudio.h>
#include <wchar.h>
#import <UIKit/UIKit.h>
#include <iron_video.h>
#import <AudioToolbox/AudioToolbox.h>
#include <mach/mach_time.h>

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

int kinc_window_width() {
	return backingWidth;
}

int kinc_window_height() {
	return backingHeight;
}

@implementation GLView

+ (Class)layerClass {
	return [CAMetalLayer class];
}

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

- (void)begin {
}

- (void)end {
}

void kinc_internal_call_resize_callback(int width, int height);

- (void)layoutSubviews {
	backingWidth = self.frame.size.width * self.contentScaleFactor;
	backingHeight = self.frame.size.height * self.contentScaleFactor;

	CAMetalLayer *metalLayer = (CAMetalLayer *)self.layer;
	metalLayer.drawableSize = CGSizeMake(backingWidth, backingHeight);

	kinc_internal_call_resize_callback(backingWidth, backingHeight);
}

- (void)dealloc {
}

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

static GLView *glView;
static bool visible;

void beginGL(void) {
	if (!visible) {
		return;
	}
	[glView begin];
}

void endGL(void) {
	if (!visible) {
		return;
	}
	[glView end];
}

void showKeyboard(void) {
	[glView showKeyboard];
}

void hideKeyboard(void) {
	[glView hideKeyboard];
}

CAMetalLayer *getMetalLayer(void) {
	return [glView metalLayer];
}

id getMetalDevice(void) {
	return [glView metalDevice];
}

id getMetalLibrary(void) {
	return [glView metalLibrary];
}

id getMetalQueue(void) {
	return [glView metalQueue];
}

@implementation GLViewController

- (void)loadView {
	visible = true;
	self.view = glView = [[GLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	[self.view addInteraction: [[UIDropInteraction alloc] initWithDelegate: self]];
	[self setNeedsUpdateOfScreenEdgesDeferringSystemGestures];
}

- (void)setVisible:(BOOL)value {
	visible = value;
}

#include <iron_system.h>

extern char mobile_title[1024];

void importFile(NSURL *url) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *folderName = [NSString stringWithUTF8String:mobile_title];
	NSString *filePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:folderName];
	if (![[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
		[[NSFileManager defaultManager] createDirectoryAtPath:filePath withIntermediateDirectories:NO attributes:nil error:nil];
	}
	NSString *suggestedName = url.path.lastPathComponent;
	filePath = [filePath stringByAppendingPathComponent:suggestedName];
	CFURLRef cfurl = (__bridge CFURLRef)url;
	CFURLStartAccessingSecurityScopedResource(cfurl);
	[[NSFileManager defaultManager] copyItemAtPath:url.path toPath:filePath error:nil];
	CFURLStopAccessingSecurityScopedResource(cfurl);
	wchar_t *wpath = (wchar_t *)[filePath cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
	kinc_internal_drop_files_callback(wpath);
}

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentAtURL:(NSURL *)url {
	// wchar_t *filePath = (wchar_t *)[url.path cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
	// CFURLRef cfurl = (__bridge CFURLRef)url;
	// CFURLStartAccessingSecurityScopedResource(cfurl);
	// kinc_internal_drop_files_callback(filePath);
	// CFURLStopAccessingSecurityScopedResource(cfurl);
	importFile(url);
}

- (void)dropInteraction:(UIDropInteraction *)interaction performDrop:(id<UIDropSession>)session {
	CGPoint point = [session locationInView:self.view];
	float x = point.x * glView.contentScaleFactor;
	float y = point.y * glView.contentScaleFactor;
	kinc_internal_mouse_trigger_move(0, x, y);
	kinc_internal_surface_trigger_move(0, x, y);

	for (UIDragItem *item in session.items) {
		[item.itemProvider loadInPlaceFileRepresentationForTypeIdentifier:item.itemProvider.registeredTypeIdentifiers[0] completionHandler:^(NSURL * _Nullable url, BOOL isInPlace, NSError * _Nullable error) {
			importFile(url);
		}];
	}
}

- (UIDropProposal *)dropInteraction:(UIDropInteraction *)interaction sessionDidUpdate:(id<UIDropSession>)session {
	return [[UIDropProposal alloc] initWithDropOperation: UIDropOperationCopy];
}

- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures
{
    return UIRectEdgeAll;
}

@end

@implementation KoreAppDelegate

static UIWindow *window;
static GLViewController *glViewController;

void loadURL(const char *url) {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

- (void)mainLoop {
	@autoreleasepool {
		kickstart(0, NULL);
	}
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	AVAudioSession *sessionInstance = [AVAudioSession sharedInstance];
	NSError *error;

	// set the session category
	NSString *category = AVAudioSessionCategoryAmbient;
	bool success = [sessionInstance setCategory:category error:&error];
	if (!success)
		NSLog(@"Error setting AVAudioSession category! %@\n", [error localizedDescription]);

	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[window setBackgroundColor:[UIColor blackColor]];

	glViewController = [[GLViewController alloc] init];
	glViewController.view.multipleTouchEnabled = YES;

	[window setRootViewController:glViewController];
	[window makeKeyAndVisible];

	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];

	return YES;
}

void KoreUpdateKeyboard(void);

- (void)applicationWillEnterForeground:(UIApplication *)application {
	[glViewController setVisible:YES];
	kinc_internal_foreground_callback();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	kinc_internal_resume_callback();
}

- (void)applicationWillResignActive:(UIApplication *)application {
	kinc_internal_pause_callback();
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	[glViewController setVisible:NO];
	kinc_internal_background_callback();
}

- (void)applicationWillTerminate:(UIApplication *)application {
	kinc_internal_shutdown_callback();
}

@end

void kinc_display_init(void) {}

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
	kinc_display_mode_t dm;
	dm.width = kinc_window_width();
	dm.height = kinc_window_height();
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
	kinc_display_mode_t dm;
	dm.width = kinc_window_width();
	dm.height = kinc_window_height();
	dm.frequency = (int)[[UIScreen mainScreen] maximumFramesPerSecond];
	dm.bits_per_pixel = 32;
	return dm;
}

int kinc_count_displays(void) {
	return 1;
}

int kinc_primary_display(void) {
	return 0;
}

@implementation Motion

@end

void kinc_internal_mouse_lock(void) {}

void kinc_internal_mouse_unlock(void) {}

bool kinc_mouse_can_lock(void) {
	return false;
}

void kinc_mouse_show(void) {}

void kinc_mouse_hide(void) {}

void kinc_mouse_set_position(int x, int y) {}

void kinc_mouse_get_position(int *x, int *y) {}

void kinc_mouse_set_cursor(int cursor_index) {}

bool withAutoreleasepool(bool (*f)(void)) {
	@autoreleasepool {
		return f();
	}
}

static bool keyboardshown = false;

const char *iphonegetresourcepath(void) {
	return [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:1];
}

bool kinc_internal_handle_messages(void) {
	SInt32 result;
	do {
		result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
	} while (result == kCFRunLoopRunHandledSource);
	return true;
}

void kinc_set_keep_screen_on(bool on) {}

void showKeyboard(void);
void hideKeyboard(void);

void kinc_keyboard_show(void) {
	keyboardshown = true;
	showKeyboard();
}

void kinc_keyboard_hide(void) {
	keyboardshown = false;
	hideKeyboard();
}

bool kinc_keyboard_active(void) {
	return keyboardshown;
}

void loadURL(const char *url);

void kinc_load_url(const char *url) {
	loadURL(url);
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

void KoreUpdateKeyboard(void) {
	if (keyboardshown) {
		hideKeyboard();
		showKeyboard();
	}
	else {
		hideKeyboard();
	}
}

void kinc_internal_shutdown(void) {}

void kinc_init(const char *name, int width, int height, struct kinc_window_options *win) {
	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_g5_internal_init();
	kinc_g4_internal_init_window(win->depth_bits, true);
}

void endGL(void);

void swapBuffersiOS(void) {
	endGL();
}

static char sysid[512];

const char *kinc_system_id(void) {
	const char *name = [[[UIDevice currentDevice] name] UTF8String];
	const char *vendorId = [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String];
	strcpy(sysid, name);
	strcat(sysid, "-");
	strcat(sysid, vendorId);
	return sysid;
}

static const char *getSavePath(void) {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *resolvedPath = [paths objectAtIndex:0];
	NSString *appName = [NSString stringWithUTF8String:kinc_application_name()];
	resolvedPath = [resolvedPath stringByAppendingPathComponent:appName];

	NSFileManager *fileMgr = [[NSFileManager alloc] init];

	NSError *error;
	[fileMgr createDirectoryAtPath:resolvedPath withIntermediateDirectories:YES attributes:nil error:&error];

	resolvedPath = [resolvedPath stringByAppendingString:@"/"];
	return [resolvedPath cStringUsingEncoding:1];
}

const char *kinc_internal_save_path(void) {
	return getSavePath();
}

static const char *videoFormats[] = {"mp4", NULL};

const char **kinc_video_formats(void) {
	return videoFormats;
}

double kinc_frequency(void) {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return (double)info.denom / (double)info.numer / 1e-9;
}

kinc_ticks_t kinc_timestamp(void) {
	kinc_ticks_t time = mach_absolute_time();
	return time;
}

const char *kinc_gamepad_vendor(int gamepad) {
	return "nobody";
}

const char *kinc_gamepad_product_name(int gamepad) {
	return "none";
}

bool kinc_gamepad_connected(int num) {
	return true;
}

void kinc_gamepad_rumble(int gamepad, float left, float right) {}

int main(int argc, char *argv[]) {
	int retVal = 0;
	@autoreleasepool {
		[KoreAppDelegate description]; // otherwise removed by the linker
		retVal = UIApplicationMain(argc, argv, nil, @"KoreAppDelegate");
	}
	return retVal;
}

static void (*resizeCallback)(int x, int y, void *data) = NULL;
static void *resizeCallbackData = NULL;

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

void kinc_window_create(kinc_window_options_t *win) {}

void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {
	resizeCallback = callback;
	resizeCallbackData = data;
}

void kinc_internal_call_resize_callback(int width, int height) {
	if (resizeCallback != NULL) {
		resizeCallback(width, height, resizeCallbackData);
	}
}

void kinc_window_set_close_callback(bool (*callback)(void *), void *data) {}

kinc_window_mode_t kinc_window_get_mode() {
	return KINC_WINDOW_MODE_FULLSCREEN;
}

int kinc_window_display() {
	return 0;
}
