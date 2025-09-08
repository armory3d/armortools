#import "ios_file_dialog.h"

#import <UIKit/UIKit.h>

static UIDocumentPickerViewController *documentPicker = nil;

void IOSFileDialogOpen() {
	UIViewController<UIDocumentPickerDelegate> *myViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
	if (documentPicker == nil) {
		documentPicker = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:@[@"public.data"] inMode:UIDocumentPickerModeOpen];
		documentPicker.delegate = myViewController;
		documentPicker.modalPresentationStyle = UIModalPresentationFormSheet;
	}
	[myViewController presentViewController:documentPicker animated:YES completion:nil];
}

wchar_t* IOSFileDialogSave() {
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	NSString *fileName = @"/untitled";
	NSString *filePath = [documentsDirectory stringByAppendingString:fileName];
	return (wchar_t*)[filePath cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
}

void IOSDeleteFile(const char *path) {
	NSError *error = nil;
	NSString *nspath = [NSString stringWithUTF8String:path];
	[[NSFileManager defaultManager] removeItemAtPath:nspath error:&error];
}

void IOSCreateDirectory(const char *path) {
	NSError *error = nil;
	NSString *nspath = [NSString stringWithUTF8String:path];
	[[NSFileManager defaultManager] createDirectoryAtPath:nspath withIntermediateDirectories:YES attributes:nil error:&error];
}
