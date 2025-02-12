#import <Cocoa/Cocoa.h>
#include <kinc/display.h>
#include <kinc/log.h>

#define maxDisplays 10

int kinc_count_displays(void) {
	NSArray *screens = [NSScreen screens];
	return (int)[screens count];
}

int kinc_primary_display(void) {
	NSArray *screens = [NSScreen screens];
	NSScreen *mainScreen = [NSScreen mainScreen];
	for (int i = 0; i < maxDisplays; ++i) {
		if (mainScreen == screens[i]) {
			return i;
		}
	}
	return -1;
}

void kinc_display_init(void) {}

kinc_display_mode_t kinc_display_available_mode(int display, int mode) {
	kinc_display_mode_t dm;
	dm.width = 800;
	dm.height = 600;
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
	NSArray *screens = [NSScreen screens];
	NSScreen *screen = screens[display];
	NSRect screenRect = [screen frame];
	kinc_display_mode_t dm;
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
