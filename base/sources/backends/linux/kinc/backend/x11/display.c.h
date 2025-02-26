
#include "x11.h"
#include <string.h>

void kinc_x11_display_init(void) {
	int eventBase;
	int errorBase;

	bool hasXinerama = (xlib.XineramaQueryExtension(x11_ctx.display, &eventBase, &errorBase) && xlib.XineramaIsActive(x11_ctx.display));
	XineramaScreenInfo *xinerama_screens = NULL;
	int xinerama_screen_count = 0;
	if (hasXinerama) {
		xinerama_screens = xlib.XineramaQueryScreens(x11_ctx.display, &xinerama_screen_count);
	}

	Window root_window = RootWindow(x11_ctx.display, DefaultScreen(x11_ctx.display));
	XRRScreenResources *screen_resources = xlib.XRRGetScreenResourcesCurrent(x11_ctx.display, root_window);
	RROutput primary_output = xlib.XRRGetOutputPrimary(x11_ctx.display, root_window);

	for (int i = 0; i < screen_resources->noutput; i++) {
		if (i >= MAXIMUM_DISPLAYS) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Too many screens (maximum %i)", MAXIMUM_DISPLAYS);
			break;
		}

		XRROutputInfo *output_info = xlib.XRRGetOutputInfo(x11_ctx.display, screen_resources, screen_resources->outputs[i]);
		if (output_info->connection != RR_Connected || output_info->crtc == None) {
			xlib.XRRFreeOutputInfo(output_info);
			continue;
		}

		XRRCrtcInfo *crtc_info = xlib.XRRGetCrtcInfo(x11_ctx.display, screen_resources, output_info->crtc);

		struct kinc_x11_display *display = &x11_ctx.displays[x11_ctx.num_displays++];
		display->index = i;
		strncpy(display->name, output_info->name, sizeof(display->name));
		display->x = crtc_info->x;
		display->y = crtc_info->y;
		display->width = crtc_info->width;
		display->height = crtc_info->height;
		display->primary = screen_resources->outputs[i] == primary_output;
		display->crtc = output_info->crtc;
		display->output = screen_resources->outputs[i];

		xlib.XRRFreeOutputInfo(output_info);
		xlib.XRRFreeCrtcInfo(crtc_info);
	}

	xlib.XRRFreeScreenResources(screen_resources);
	if (hasXinerama) {
		xlib.XFree(xinerama_screens);
	}
}

int kinc_x11_display_primary(void) {
	for (int i = 0; i < x11_ctx.num_displays; i++) {
		if (x11_ctx.displays[i].primary) {
			return i;
		}
	}
	return 0;
}

int kinc_x11_count_displays(void) {
	return x11_ctx.num_displays;
}

bool kinc_x11_display_available(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS) {
		return false;
	}

	return x11_ctx.displays[display_index].output != None;
}

const char *kinc_x11_display_name(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS) {
		return "";
	}

	return x11_ctx.displays[display_index].name;
}

kinc_display_mode_t kinc_x11_display_current_mode(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_x11_display *display = &x11_ctx.displays[display_index];
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = display->width;
	mode.height = display->height;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	mode.pixels_per_inch = 96;

	Window root_window = DefaultRootWindow(x11_ctx.display);
	XRRScreenResources *screen_resources = xlib.XRRGetScreenResourcesCurrent(x11_ctx.display, root_window);

	XRROutputInfo *output_info = xlib.XRRGetOutputInfo(x11_ctx.display, screen_resources, screen_resources->outputs[display->index]);
	if (output_info->connection != RR_Connected || output_info->crtc == None) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Display %i not connected.", display_index);
		xlib.XRRFreeOutputInfo(output_info);
		xlib.XRRFreeScreenResources(screen_resources);
		return mode;
	}

	XRRCrtcInfo *crtc_info = xlib.XRRGetCrtcInfo(x11_ctx.display, screen_resources, output_info->crtc);
	for (int j = 0; j < output_info->nmode; j++) {
		RRMode rr_mode = crtc_info->mode;
		XRRModeInfo *mode_info = NULL;
		for (int k = 0; k < screen_resources->nmode; k++) {
			if (screen_resources->modes[k].id == rr_mode) {
				mode_info = &screen_resources->modes[k];
				break;
			}
		}

		if (mode_info == NULL) {
			continue;
		}
		mode.x = display->x;
		mode.y = display->y;
		mode.width = mode_info->width;
		mode.height = mode_info->height;
		mode.pixels_per_inch = 96;
		mode.bits_per_pixel = 32;
		if (mode_info->hTotal && mode_info->vTotal) {
			mode.frequency = (mode_info->dotClock / (mode_info->hTotal * mode_info->vTotal));
		}
		else {
			mode.frequency = 60;
		}
	}
	xlib.XRRFreeOutputInfo(output_info);
	xlib.XRRFreeCrtcInfo(crtc_info);
	xlib.XRRFreeScreenResources(screen_resources);
	return mode;
}

int kinc_x11_display_count_available_modes(int display_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_x11_display *display = &x11_ctx.displays[display_index];

	Window root_window = RootWindow(x11_ctx.display, DefaultScreen(x11_ctx.display));
	XRRScreenResources *screen_resources = xlib.XRRGetScreenResourcesCurrent(x11_ctx.display, root_window);

	XRROutputInfo *output_info = xlib.XRRGetOutputInfo(x11_ctx.display, screen_resources, screen_resources->outputs[display->index]);
	if (output_info->connection != RR_Connected || output_info->crtc == None) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Display %i not connected.", display_index);
		xlib.XRRFreeOutputInfo(output_info);
		xlib.XRRFreeScreenResources(screen_resources);
		return 0;
	}

	int num_modes = output_info->nmode;
	xlib.XRRFreeOutputInfo(output_info);
	xlib.XRRFreeScreenResources(screen_resources);
	return num_modes;
}

kinc_display_mode_t kinc_x11_display_available_mode(int display_index, int mode_index) {
	if (display_index >= MAXIMUM_DISPLAYS)
		display_index = 0;
	struct kinc_x11_display *display = &x11_ctx.displays[display_index];
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = display->width;
	mode.height = display->height;
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	mode.pixels_per_inch = 96;

	Window root_window = RootWindow(x11_ctx.display, DefaultScreen(x11_ctx.display));
	XRRScreenResources *screen_resources = xlib.XRRGetScreenResourcesCurrent(x11_ctx.display, root_window);

	XRROutputInfo *output_info = xlib.XRRGetOutputInfo(x11_ctx.display, screen_resources, screen_resources->outputs[display->index]);
	if (output_info->connection != RR_Connected || output_info->crtc == None) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Display %i not connected.", display_index);
		xlib.XRRFreeOutputInfo(output_info);
		xlib.XRRFreeScreenResources(screen_resources);
		return mode;
	}

	if (mode_index >= output_info->nmode) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Invalid mode index %i.", mode_index);
	}

	RRMode rr_mode = output_info->modes[mode_index];
	XRRModeInfo *mode_info = NULL;

	for (int k = 0; k < screen_resources->nmode; k++) {
		if (screen_resources->modes[k].id == rr_mode) {
			mode_info = &screen_resources->modes[k];
			break;
		}
	}

	if (mode_info != NULL) {
		mode.x = display->x;
		mode.y = display->y;
		mode.width = mode_info->width;
		mode.height = mode_info->height;
		mode.pixels_per_inch = 96;
		mode.bits_per_pixel = 32;
		if (mode_info->hTotal && mode_info->vTotal) {
			mode.frequency = (mode_info->dotClock / (mode_info->hTotal * mode_info->vTotal));
		}
		else {
			mode.frequency = 60;
		}
	}

	xlib.XRRFreeOutputInfo(output_info);
	xlib.XRRFreeScreenResources(screen_resources);
	return mode;
}
