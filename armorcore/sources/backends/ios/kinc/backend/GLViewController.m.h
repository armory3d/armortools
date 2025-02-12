#import "GLView.h"
#import "GLViewController.h"
#import <Foundation/Foundation.h>
#include <kinc/graphics5/rendertarget.h>
#include <kinc/math/core.h>
#include <objc/runtime.h>

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

#include <kinc/system.h>
#include <kinc/input/mouse.h>
#include <kinc/input/surface.h>

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
