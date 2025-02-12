#include <kinc/backend/windows.h>
#include <kinc/display.h>
#include <kinc/error.h>
#include <kinc/log.h>

#undef RegisterClass

#define MAXIMUM_DISPLAYS 16

typedef struct {
	struct HMONITOR__ *monitor;
	char name[32];
	bool primary, available, mode_changed;
	int index, x, y, width, height, ppi, frequency, bpp;
} DisplayData;

static DisplayData displays[MAXIMUM_DISPLAYS];
static DEVMODEA original_modes[MAXIMUM_DISPLAYS];
static int screen_counter = 0;
static bool display_initialized = false;

typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
typedef HRESULT(WINAPI *GetDpiForMonitorType)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);
static GetDpiForMonitorType MyGetDpiForMonitor = NULL;

static BOOL CALLBACK EnumerationCallback(HMONITOR monitor, HDC hdc_unused, LPRECT rect_unused, LPARAM lparam) {
	MONITORINFOEXA info;
	memset(&info, 0, sizeof(MONITORINFOEXA));
	info.cbSize = sizeof(MONITORINFOEXA);

	if (GetMonitorInfoA(monitor, (MONITORINFO *)&info) == FALSE) {
		return FALSE;
	}

	int free_slot = 0;
	for (; free_slot < MAXIMUM_DISPLAYS; ++free_slot) {
		if (displays[free_slot].monitor == monitor) {
			return FALSE;
		}

		if (displays[free_slot].monitor == NULL) {
			break;
		}
	}

	DisplayData *display = &displays[free_slot];
	strncpy(display->name, info.szDevice, 31);
	display->name[31] = 0;
	display->index = free_slot;
	display->monitor = monitor;
	display->primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
	display->available = true;
	display->x = info.rcMonitor.left;
	display->y = info.rcMonitor.top;
	display->width = info.rcMonitor.right - info.rcMonitor.left;
	display->height = info.rcMonitor.bottom - info.rcMonitor.top;

	HDC hdc = CreateDCA(NULL, display->name, NULL, NULL);
	display->ppi = GetDeviceCaps(hdc, LOGPIXELSX);
	int scale = GetDeviceCaps(hdc, SCALINGFACTORX);
	DeleteDC(hdc);

	if (MyGetDpiForMonitor != NULL) {
		unsigned dpiX, dpiY;
		MyGetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
		display->ppi = (int)dpiX;
	}

	memset(&original_modes[free_slot], 0, sizeof(DEVMODEA));
	original_modes[free_slot].dmSize = sizeof(DEVMODEA);
	EnumDisplaySettingsA(display->name, ENUM_CURRENT_SETTINGS, &original_modes[free_slot]);
	display->frequency = original_modes[free_slot].dmDisplayFrequency;
	display->bpp = original_modes[free_slot].dmBitsPerPel;

	++screen_counter;
	return TRUE;
}

void kinc_display_init() {
	if (display_initialized) {
		return;
	}
	HMODULE shcore = LoadLibraryA("Shcore.dll");
	if (shcore != NULL) {
		MyGetDpiForMonitor = (GetDpiForMonitorType)GetProcAddress(shcore, "GetDpiForMonitor");
	}
	memset(displays, 0, sizeof(DisplayData) * MAXIMUM_DISPLAYS);
	EnumDisplayMonitors(NULL, NULL, EnumerationCallback, 0);
	display_initialized = true;
}

int kinc_windows_get_display_for_monitor(struct HMONITOR__ *monitor) {
	for (int i = 0; i < MAXIMUM_DISPLAYS; ++i) {
		if (displays[i].monitor == monitor) {
			return i;
		}
	}

	kinc_error_message("Display for monitor not found");
	return -1;
}

int kinc_count_displays() {
	return screen_counter;
}

int kinc_primary_display() {
	for (int i = 0; i < MAXIMUM_DISPLAYS; ++i) {
		DisplayData *display = &displays[i];

		if (display->available && display->primary) {
			return i;
		}
	}

	kinc_error_message("No primary display defined");
	return -1;
}

kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index) {
	DEVMODEA dev_mode = {0};
	dev_mode.dmSize = sizeof(DEVMODEA);
	EnumDisplaySettingsA(displays[display_index].name, mode_index, &dev_mode);
	kinc_display_mode_t mode;
	mode.x = displays[display_index].x;
	mode.y = displays[display_index].y;
	mode.width = dev_mode.dmPelsWidth;
	mode.height = dev_mode.dmPelsHeight;
	mode.frequency = dev_mode.dmDisplayFrequency;
	mode.bits_per_pixel = dev_mode.dmBitsPerPel;
	mode.pixels_per_inch = displays[display_index].ppi * mode.width / original_modes[display_index].dmPelsWidth;
	return mode;
}

int kinc_display_count_available_modes(int display_index) {
	DEVMODEA dev_mode = {0};
	dev_mode.dmSize = sizeof(DEVMODEA);
	int i = 0;
	for (; EnumDisplaySettingsA(displays[display_index].name, i, &dev_mode) != FALSE; ++i)
		;
	return i;
}

bool kinc_windows_set_display_mode(int display_index, int width, int height, int bpp, int frequency) {
	DisplayData *display = &displays[display_index];
	display->mode_changed = true;
	DEVMODEA mode = {0};
	mode.dmSize = sizeof(mode);
	strcpy((char *)mode.dmDeviceName, display->name);
	mode.dmPelsWidth = width;
	mode.dmPelsHeight = height;
	mode.dmBitsPerPel = bpp;
	mode.dmDisplayFrequency = frequency;
	mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

	bool success = ChangeDisplaySettingsA(&mode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;

	if (!success) {
		mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		success = ChangeDisplaySettingsA(&mode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
	}

	return success;
}

void kinc_windows_restore_display(int display) {
	if (displays[display].mode_changed) {
		ChangeDisplaySettingsA(&original_modes[display], 0);
	}
}

void kinc_windows_restore_displays() {
	for (int i = 0; i < MAXIMUM_DISPLAYS; ++i) {
		kinc_windows_restore_display(i);
	}
}

bool kinc_display_available(int display_index) {
	if (display_index < 0 || display_index >= MAXIMUM_DISPLAYS) {
		return false;
	}
	return displays[display_index].available;
}

const char *kinc_display_name(int display_index) {
	return displays[display_index].name;
}

kinc_display_mode_t kinc_display_current_mode(int display_index) {
	DisplayData *display = &displays[display_index];
	kinc_display_mode_t mode;
	mode.x = display->x;
	mode.y = display->y;
	mode.width = display->width;
	mode.height = display->height;
	mode.pixels_per_inch = display->ppi;
	mode.frequency = display->frequency;
	mode.bits_per_pixel = display->bpp;
	return mode;
}
