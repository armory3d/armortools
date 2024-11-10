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
	// NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	// try {
	@autoreleasepool {
		kickstart(0, NULL);
	}
	//}
	// catch (Kt::Exception& ex) {
	//	printf("Exception\n");
	//	printf("%s", ex.what());
	//}

	//[pool drain];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	AVAudioSession *sessionInstance = [AVAudioSession sharedInstance];
	NSError *error;

	// set the session category
	NSString *category = AVAudioSessionCategoryAmbient;
	bool success = [sessionInstance setCategory:category error:&error];
	if (!success)
		NSLog(@"Error setting AVAudioSession category! %@\n", [error localizedDescription]);
	// CGRect rect = [[UIScreen mainScreen] applicationFrame];
	// CGRect screenBounds = [[UIScreen mainScreen] bounds];

	// window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, Kore::max(screenBounds.size.width, screenBounds.size.height),
	// Kore::max(screenBounds.size.width, screenBounds.size.height))];
	// CGRect bounds = [[UIScreen mainScreen] bounds];
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[window setBackgroundColor:[UIColor blackColor]];

	// glView = [[GLView alloc] initWithFrame:CGRectMake(0, 0, Kore::max(screenBounds.size.width, screenBounds.size.height), Kore::max(screenBounds.size.width,
	// screenBounds.size.height))];
	glViewController = [[GLViewController alloc] init];
#ifndef KINC_TVOS
	glViewController.view.multipleTouchEnabled = YES;
#endif
	// glViewController.view = glView;
	//[glViewController ]
	//[window addSubview:glView];
	[window setRootViewController:glViewController];
	[window makeKeyAndVisible];

	[self performSelectorOnMainThread:@selector(mainLoop) withObject:nil waitUntilDone:NO];

	return YES;
}

#ifndef KINC_TVOS
// static Kore::Orientation convertOrientation(UIDeviceOrientation orientation) {
//	switch (orientation) {
//	case UIDeviceOrientationLandscapeLeft:
//		return Kore::OrientationLandscapeRight;
//	case UIDeviceOrientationLandscapeRight:
//		return Kore::OrientationLandscapeLeft;
//	case UIDeviceOrientationPortrait:
//		return Kore::OrientationPortrait;
//	case UIDeviceOrientationPortraitUpsideDown:
//		return Kore::OrientationPortraitUpsideDown;
//	default:
//		return Kore::OrientationUnknown;
//	}
//}

// static UIInterfaceOrientation convertAppleOrientation(UIDeviceOrientation orientation, UIInterfaceOrientation lastOrientation) {
//	switch (orientation) {
//	case UIDeviceOrientationLandscapeLeft:
//		return UIInterfaceOrientationLandscapeRight;
//	case UIDeviceOrientationLandscapeRight:
//		return UIInterfaceOrientationLandscapeLeft;
//	case UIDeviceOrientationPortrait:
//		return UIInterfaceOrientationPortrait;
//	case UIDeviceOrientationPortraitUpsideDown:
//		return UIInterfaceOrientationPortraitUpsideDown;
//	default:
//		return lastOrientation;
//	}
//}
#endif

void KoreUpdateKeyboard(void);
/*
- (void)didRotate:(NSNotification*)notification {
    if (Kore::Application::the() != nullptr && Kore::Application::the()->orientationCallback != nullptr)
Kore::Application::the()->orientationCallback(convertOrientation([[UIDevice currentDevice] orientation]));
    [UIApplication sharedApplication].statusBarOrientation = convertAppleOrientation([[UIDevice currentDevice] orientation], [UIApplication
sharedApplication].statusBarOrientation);
    KoreUpdateKeyboard();
}
*/
- (void)applicationWillEnterForeground:(UIApplication *)application {
	[glViewController setVisible:YES];
	kinc_internal_foreground_callback();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	kinc_internal_resume_callback();
	//[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	//[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	kinc_internal_pause_callback();
	//[[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	[glViewController setVisible:NO];
	kinc_internal_background_callback();
}

- (void)applicationWillTerminate:(UIApplication *)application {
	kinc_internal_shutdown_callback();
}

//- (void)dealloc {
//    [window release];
//    [glView release];
//    [super dealloc];
//}

@end
