#import "BasicOpenGLView.h"
#import <Cocoa/Cocoa.h>
#include <kinc/backend/HIDManager.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/input/keyboard.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include "windowdata.h"
#include <kinc/backend/windowdata.h>

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

void swapBuffersMac(int windowId) {
}

static int createWindow(kinc_window_options_t *options) {
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

	windows[windowCounter].handle = window;
	windows[windowCounter].view = view;

	[window makeKeyAndOrderFront:nil];

	if (options->mode == KINC_WINDOW_MODE_FULLSCREEN) {
		[window toggleFullScreen:nil];
		windows[windowCounter].fullscreen = true;
	}

	return windowCounter++;
}

int kinc_count_windows(void) {
	return windowCounter;
}

void kinc_window_change_window_mode(int window_index, kinc_window_mode_t mode) {
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW:
		if (windows[window_index].fullscreen) {
			[window toggleFullScreen:nil];
			windows[window_index].fullscreen = false;
		}
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
		if (!windows[window_index].fullscreen) {
			[window toggleFullScreen:nil];
			windows[window_index].fullscreen = true;
		}
		break;
	}
}

void kinc_window_set_close_callback(int window, bool (*callback)(void *), void *data) {
	windows[window].closeCallback = callback;
	windows[window].closeCallbackData = data;
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

int kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
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

	int windowId = createWindow(win);
	kinc_g4_internal_init();
	kinc_g4_internal_init_window(windowId, frame->depth_bits, true);

	return 0;
}

int kinc_window_width(int window_index) {
	NSWindow *window = windows[window_index].handle;
	float scale = [window backingScaleFactor];
	return [[window contentView] frame].size.width * scale;
}

int kinc_window_height(int window_index) {
	NSWindow *window = windows[window_index].handle;
	float scale = [window backingScaleFactor];
	return [[window contentView] frame].size.height * scale;
}

NSWindow *kinc_get_mac_window_handle(int window_index) {
	return windows[window_index].handle;
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
