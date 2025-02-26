#import "ios_file_dialog.h"

#import <UIKit/UIKit.h>

void IOSFileDialogOpen() {
	UIViewController<UIDocumentPickerDelegate> *glViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
	UIDocumentPickerViewController *documentPicker = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:@[@"public.data"] inMode:UIDocumentPickerModeOpen];
	documentPicker.delegate = glViewController;
	documentPicker.modalPresentationStyle = UIModalPresentationFormSheet;
	[glViewController presentViewController:documentPicker animated:YES completion:nil];
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
