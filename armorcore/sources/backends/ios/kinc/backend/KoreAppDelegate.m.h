#import "GLView.h"
#import "GLViewController.h"
#import "KoreAppDelegate.h"
#import <AVFAudio/AVFAudio.h>
#include <kinc/system.h>
#include <wchar.h>

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
