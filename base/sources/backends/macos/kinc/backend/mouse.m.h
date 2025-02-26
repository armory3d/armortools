#import <Cocoa/Cocoa.h>

#include <kinc/backend/windowdata.h>
#include <iron_input.h>
#include <iron_system.h>

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
