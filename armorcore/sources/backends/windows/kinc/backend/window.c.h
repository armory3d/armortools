#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>
#include <kinc/backend/windows.h>

#undef CreateWindow

struct HWND__;
typedef unsigned long DWORD;

// Dark mode
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
struct HWND__ *kinc_windows_window_handle(int window_index);
// Enable visual styles for ui controls
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

typedef struct {
	struct HWND__ *handle;
	int display_index;
	bool mouseInside;
	int index;
	int x, y, mode, bpp, frequency, features;
	int manualWidth, manualHeight;
	void (*resizeCallback)(int x, int y, void *data);
	void *resizeCallbackData;
	bool (*closeCallback)(void *data);
	void *closeCallbackData;
} WindowData;

LRESULT WINAPI KoreWindowsMessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define MAXIMUM_WINDOWS 16
static WindowData windows[MAXIMUM_WINDOWS] = {0};
static int window_counter = 0;

const wchar_t *windowClassName = L"KoreWindow";

static void RegisterWindowClass(HINSTANCE hInstance, const wchar_t *className) {
	WNDCLASSEXW wc = {sizeof(WNDCLASSEXA),
	                  CS_OWNDC /*CS_CLASSDC*/,
	                  KoreWindowsMessageProcedure,
	                  0L,
	                  0L,
	                  hInstance,
	                  LoadIconW(hInstance, MAKEINTRESOURCEW(107)),
	                  LoadCursor(NULL, IDC_ARROW),
	                  CreateSolidBrush(0x00202020),
	                  0,
	                  className,
	                  0};
	RegisterClassExW(&wc);
}

static DWORD getStyle(int features) {
	DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;

	if ((features & KINC_WINDOW_FEATURE_RESIZEABLE) && ((features & KINC_WINDOW_FEATURE_BORDERLESS) == 0)) {
		style |= WS_SIZEBOX;
	}

	if (features & KINC_WINDOW_FEATURE_MAXIMIZABLE) {
		style |= WS_MAXIMIZEBOX;
	}

	if (features & KINC_WINDOW_FEATURE_MINIMIZABLE) {
		style |= WS_MINIMIZEBOX;
	}

	if ((features & KINC_WINDOW_FEATURE_BORDERLESS) == 0) {
		style |= WS_CAPTION | WS_SYSMENU;
	}

	return style;
}

static DWORD getExStyle(int features) {
	DWORD exStyle = WS_EX_APPWINDOW;

	if ((features & KINC_WINDOW_FEATURE_BORDERLESS) == 0) {
		exStyle |= WS_EX_WINDOWEDGE;
	}

	if (features & KINC_WINDOW_FEATURE_ON_TOP) {
		exStyle |= WS_EX_TOPMOST;
	}

	return exStyle;
}

int kinc_windows_window_index_from_hwnd(struct HWND__ *handle) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		if (windows[i].handle == handle) {
			return i;
		}
	}
	return -1;
}

int kinc_count_windows() {
	return window_counter;
}

int kinc_window_x(int window_index) {
	RECT rect;
	GetWindowRect(windows[window_index].handle, &rect);
	windows[window_index].x = rect.left;
	return windows[window_index].x;
}

int kinc_window_y(int window_index) {
	RECT rect;
	GetWindowRect(windows[window_index].handle, &rect);
	windows[window_index].y = rect.top;
	return windows[window_index].y;
}

int kinc_window_width(int window_index) {
	RECT rect;
	GetClientRect(windows[window_index].handle, &rect);
	return rect.right;
}

int kinc_window_height(int window_index) {
	RECT rect;
	GetClientRect(windows[window_index].handle, &rect);
	return rect.bottom;
}

static DWORD getDwStyle(kinc_window_mode_t mode, int features) {
	switch (mode) {
	case KINC_WINDOW_MODE_FULLSCREEN:
		return WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	case KINC_WINDOW_MODE_WINDOW:
	default:
		return getStyle(features);
	}
}

static DWORD getDwExStyle(kinc_window_mode_t mode, int features) {
	switch (mode) {
	case KINC_WINDOW_MODE_FULLSCREEN:
		return WS_EX_APPWINDOW;
	case KINC_WINDOW_MODE_WINDOW:
	default:
		return getExStyle(features);
	}
}

static int createWindow(const wchar_t *title, int x, int y, int width, int height, int bpp, int frequency, int features, kinc_window_mode_t windowMode,
                        int target_display_index) {
	HINSTANCE inst = GetModuleHandleW(NULL);

	if (window_counter == 0) {
		RegisterWindowClass(inst, windowClassName);
	}

	int display_index = target_display_index == -1 ? kinc_primary_display() : target_display_index;

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = width;
	WindowRect.top = 0;
	WindowRect.bottom = height;

	AdjustWindowRectEx(&WindowRect, getDwStyle(windowMode, features), FALSE, getDwExStyle(windowMode, features));

	kinc_display_mode_t display_mode = kinc_display_current_mode(display_index);

	int dstx = display_mode.x;
	int dsty = display_mode.y;
	int dstw = width;
	int dsth = height;

	switch (windowMode) {
	case KINC_WINDOW_MODE_WINDOW:
		dstx += x < 0 ? (display_mode.width - width) / 2 : x;
		dsty += y < 0 ? (display_mode.height - height) / 2 : y;
		dstw = WindowRect.right - WindowRect.left;
		dsth = WindowRect.bottom - WindowRect.top;
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
		dstw = display_mode.width;
		dsth = display_mode.height;
		break;
	}

	HWND hwnd = CreateWindowExW(getDwExStyle(windowMode, features), windowClassName, title, getDwStyle(windowMode, features), dstx, dsty, dstw, dsth, NULL, NULL,
	                            inst, NULL);

	SetCursor(LoadCursor(NULL, IDC_ARROW));
	DragAcceptFiles(hwnd, true);

	windows[window_counter].handle = hwnd;
	windows[window_counter].x = dstx;
	windows[window_counter].y = dsty;
	windows[window_counter].mode = windowMode;
	windows[window_counter].display_index = display_index;
	windows[window_counter].bpp = bpp;
	windows[window_counter].frequency = frequency;
	windows[window_counter].features = features;
	windows[window_counter].manualWidth = width;
	windows[window_counter].manualHeight = height;
	windows[window_counter].index = window_counter;

	// Dark mode
	char vdata[4];
	DWORD cbdata = 4 * sizeof(char);
	RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, vdata, &cbdata);
	BOOL use_dark_mode = (int)(vdata[3] << 24 | vdata[2] << 16 | vdata[1] << 8 | vdata[0]) != 1;
	DwmSetWindowAttribute(kinc_windows_window_handle(0), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

	return window_counter++;
}

void kinc_window_resize(int window_index, int width, int height) {
	WindowData *win = &windows[window_index];
	win->manualWidth = width;
	win->manualHeight = height;
	switch (win->mode) {
	case KINC_WINDOW_MODE_WINDOW: {
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRectEx(&rect, getDwStyle((kinc_window_mode_t)win->mode, win->features), FALSE, getDwExStyle((kinc_window_mode_t)win->mode, win->features));
		SetWindowPos(win->handle, NULL, kinc_window_x(window_index), kinc_window_y(window_index), rect.right - rect.left, rect.bottom - rect.top, 0);
		break;
	}
	}
}

void kinc_window_move(int window_index, int x, int y) {
	WindowData *win = &windows[window_index];

	if (win->mode != 0) {
		return;
	}

	win->x = x;
	win->y = y;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = kinc_window_width(window_index);
	rect.bottom = kinc_window_height(window_index);
	AdjustWindowRectEx(&rect, getDwStyle((kinc_window_mode_t)win->mode, win->features), FALSE, getDwExStyle((kinc_window_mode_t)win->mode, win->features));

	SetWindowPos(win->handle, NULL, x, y, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame);

void kinc_window_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {
	kinc_internal_change_framebuffer(window, frame);
}

void kinc_window_change_features(int window_index, int features) {
	WindowData *win = &windows[window_index];
	win->features = features;
	SetWindowLongW(win->handle, GWL_STYLE, getStyle(features));
	SetWindowLongW(win->handle, GWL_EXSTYLE, getExStyle(features));

	HWND on_top = (features & KINC_WINDOW_FEATURE_ON_TOP) ? HWND_TOPMOST : HWND_NOTOPMOST;
	SetWindowPos(win->handle, on_top, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	kinc_window_show(window_index);
}

void kinc_window_change_mode(int window_index, kinc_window_mode_t mode) {
	WindowData *win = &windows[window_index];
	int display_index = kinc_window_display(window_index);
	kinc_display_mode_t display_mode = kinc_display_current_mode(display_index);
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW: {
		kinc_windows_restore_display(display_index);
		kinc_window_change_features(window_index, win->features);
		kinc_window_show(window_index);
		break;
	}
	case KINC_WINDOW_MODE_FULLSCREEN: {
		kinc_windows_restore_display(display_index);
		SetWindowLongW(win->handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLongW(win->handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		kinc_window_show(window_index);
		break;
	}
	}
	win->mode = mode;
	DragAcceptFiles(win->handle, true);
}

kinc_window_mode_t kinc_window_get_mode(int window_index) {
	return (kinc_window_mode_t)windows[window_index].mode;
}

void kinc_window_destroy(int window_index) {
	WindowData *win = &windows[window_index];
	if (win->handle != NULL) {
		kinc_g4_internal_destroy_window(window_index);
		DestroyWindow(win->handle);
		win->handle = NULL;
		--window_counter;
	}
}

void kinc_windows_hide_windows(void) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		if (windows[i].handle != NULL) {
			ShowWindow(windows[i].handle, SW_HIDE);
			UpdateWindow(windows[i].handle);
		}
	}
}

void kinc_windows_destroy_windows(void) {
	for (int i = 0; i < MAXIMUM_WINDOWS; ++i) {
		kinc_window_destroy(i);
	}
	UnregisterClassW(windowClassName, GetModuleHandleW(NULL));
}

void kinc_window_show(int window_index) {
	ShowWindow(windows[window_index].handle, SW_SHOWDEFAULT);
	UpdateWindow(windows[window_index].handle);
}

void kinc_window_hide(int window_index) {
	ShowWindow(windows[window_index].handle, SW_HIDE);
	UpdateWindow(windows[window_index].handle);
}

void kinc_window_set_title(int window_index, const char *title) {
	wchar_t buffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, buffer, 1024);
	SetWindowTextW(windows[window_index].handle, buffer);
}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	kinc_window_options_t defaultWin;
	kinc_framebuffer_options_t defaultFrame;

	if (win == NULL) {
		kinc_window_options_set_defaults(&defaultWin);
		win = &defaultWin;
	}

	if (frame == NULL) {
		kinc_framebuffer_options_set_defaults(&defaultFrame);
		frame = &defaultFrame;
	}

	if (win->title == NULL) {
		win->title = "";
	}

	wchar_t wbuffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, win->title, -1, wbuffer, 1024);

	int windowId = createWindow(wbuffer, win->x, win->y, win->width, win->height, frame->color_bits, frame->frequency, win->window_features, win->mode,
	                            win->display_index);

	bool vsync = frame->vertical_sync;

	kinc_g4_internal_init_window(windowId, frame->depth_bits, vsync);

	if (win->visible) {
		kinc_window_show(windowId);
	}

	return windowId;
}

void kinc_window_set_resize_callback(int window_index, void (*callback)(int x, int y, void *data), void *data) {
	windows[window_index].resizeCallback = callback;
	windows[window_index].resizeCallbackData = data;
}

void kinc_window_set_close_callback(int window_index, bool (*callback)(void *data), void *data) {
	windows[window_index].closeCallback = callback;
	windows[window_index].closeCallbackData = data;
}

int kinc_window_display(int window_index) {
	return kinc_windows_get_display_for_monitor(MonitorFromWindow(windows[window_index].handle, MONITOR_DEFAULTTOPRIMARY));
}

struct HWND__ *kinc_windows_window_handle(int window_index) {
	return windows[window_index].handle;
}

int kinc_windows_manual_width(int window) {
	return windows[window].manualWidth;
}

int kinc_windows_manual_height(int window) {
	return windows[window].manualHeight;
}

void kinc_internal_call_resize_callback(int window_index, int width, int height) {
	if (windows[window_index].resizeCallback != NULL) {
		windows[window_index].resizeCallback(width, height, windows[window_index].resizeCallbackData);
	}
}

bool kinc_internal_call_close_callback(int window_index) {
	if (windows[window_index].closeCallback != NULL) {
		return windows[window_index].closeCallback(windows[window_index].closeCallbackData);
	}
	return true;
}
