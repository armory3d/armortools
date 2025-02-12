#import "KoreAppDelegate.h"
#include <kinc/graphics4/graphics.h>
#include <kinc/input/gamepad.h>
#include <kinc/input/keyboard.h>
#include <kinc/system.h>
#include <kinc/video.h>
#include <kinc/window.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>

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

int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame) {
	kinc_window_options_t defaultWin;
	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}
	kinc_framebuffer_options_t defaultFrame;
	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}
	kinc_g4_internal_init();
	kinc_g4_internal_init_window(0, frame->depth_bits, true);

	return 0;
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

#include <mach/mach_time.h>

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
