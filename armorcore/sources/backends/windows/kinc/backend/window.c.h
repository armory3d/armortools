#include <kinc/display.h>
#include <kinc/graphics5/g5.h>
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
struct HWND__ *kinc_windows_window_handle();
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

int kinc_window_x() {
	RECT rect;
	GetWindowRect(windows[0].handle, &rect);
	windows[0].x = rect.left;
	return windows[0].x;
}

int kinc_window_y() {
	RECT rect;
	GetWindowRect(windows[0].handle, &rect);
	windows[0].y = rect.top;
	return windows[0].y;
}

int kinc_window_width() {
	RECT rect;
	GetClientRect(windows[0].handle, &rect);
	return rect.right;
}

int kinc_window_height() {
	RECT rect;
	GetClientRect(windows[0].handle, &rect);
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

static void createWindow(const wchar_t *title, int x, int y, int width, int height, int bpp, int frequency, int features, kinc_window_mode_t windowMode,
                        int target_display_index) {
	HINSTANCE inst = GetModuleHandleW(NULL);
	RegisterWindowClass(inst, windowClassName);

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

	windows[0].handle = hwnd;
	windows[0].x = dstx;
	windows[0].y = dsty;
	windows[0].mode = windowMode;
	windows[0].display_index = display_index;
	windows[0].bpp = bpp;
	windows[0].frequency = frequency;
	windows[0].features = features;
	windows[0].manualWidth = width;
	windows[0].manualHeight = height;

	// Dark mode
	char vdata[4];
	DWORD cbdata = 4 * sizeof(char);
	RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, vdata, &cbdata);
	BOOL use_dark_mode = (int)(vdata[3] << 24 | vdata[2] << 16 | vdata[1] << 8 | vdata[0]) != 1;
	DwmSetWindowAttribute(kinc_windows_window_handle(0), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));
}

void kinc_window_resize(int width, int height) {
	WindowData *win = &windows[0];
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
		SetWindowPos(win->handle, NULL, kinc_window_x(), kinc_window_y(), rect.right - rect.left, rect.bottom - rect.top, 0);
		break;
	}
	}
}

void kinc_window_move(int x, int y) {
	WindowData *win = &windows[0];

	if (win->mode != 0) {
		return;
	}

	win->x = x;
	win->y = y;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = kinc_window_width();
	rect.bottom = kinc_window_height();
	AdjustWindowRectEx(&rect, getDwStyle((kinc_window_mode_t)win->mode, win->features), FALSE, getDwExStyle((kinc_window_mode_t)win->mode, win->features));

	SetWindowPos(win->handle, NULL, x, y, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void kinc_window_change_features(int features) {
	WindowData *win = &windows[0];
	win->features = features;
	SetWindowLongW(win->handle, GWL_STYLE, getStyle(features));
	SetWindowLongW(win->handle, GWL_EXSTYLE, getExStyle(features));

	HWND on_top = (features & KINC_WINDOW_FEATURE_ON_TOP) ? HWND_TOPMOST : HWND_NOTOPMOST;
	SetWindowPos(win->handle, on_top, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	kinc_window_show();
}

void kinc_window_change_mode(kinc_window_mode_t mode) {
	WindowData *win = &windows[0];
	int display_index = kinc_window_display();
	kinc_display_mode_t display_mode = kinc_display_current_mode(display_index);
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW: {
		kinc_windows_restore_display(display_index);
		kinc_window_change_features(win->features);
		kinc_window_show();
		break;
	}
	case KINC_WINDOW_MODE_FULLSCREEN: {
		kinc_windows_restore_display(display_index);
		SetWindowLongW(win->handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP);
		SetWindowLongW(win->handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowPos(win->handle, NULL, display_mode.x, display_mode.y, display_mode.width, display_mode.height, 0);
		kinc_window_show();
		break;
	}
	}
	win->mode = mode;
	DragAcceptFiles(win->handle, true);
}

kinc_window_mode_t kinc_window_get_mode() {
	return (kinc_window_mode_t)windows[0].mode;
}

void kinc_window_destroy() {
	WindowData *win = &windows[0];
	if (win->handle != NULL) {
		kinc_g5_internal_destroy_window();
		DestroyWindow(win->handle);
		win->handle = NULL;
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

void kinc_window_show() {
	ShowWindow(windows[0].handle, SW_SHOWDEFAULT);
	UpdateWindow(windows[0].handle);
}

void kinc_window_hide() {
	ShowWindow(windows[0].handle, SW_HIDE);
	UpdateWindow(windows[0].handle);
}

void kinc_window_set_title(const char *title) {
	wchar_t buffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, buffer, 1024);
	SetWindowTextW(windows[0].handle, buffer);
}

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
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

	createWindow(wbuffer, win->x, win->y, win->width, win->height, frame->color_bits, frame->frequency, win->window_features, win->mode,
	                            win->display_index);

	bool vsync = frame->vertical_sync;

	kinc_g4_internal_init_window(frame->depth_bits, vsync);

	if (win->visible) {
		kinc_window_show();
	}
}

void kinc_window_set_resize_callback(void (*callback)(int x, int y, void *data), void *data) {
	windows[0].resizeCallback = callback;
	windows[0].resizeCallbackData = data;
}

void kinc_window_set_close_callback(bool (*callback)(void *data), void *data) {
	windows[0].closeCallback = callback;
	windows[0].closeCallbackData = data;
}

int kinc_window_display() {
	return kinc_windows_get_display_for_monitor(MonitorFromWindow(windows[0].handle, MONITOR_DEFAULTTOPRIMARY));
}

struct HWND__ *kinc_windows_window_handle() {
	return windows[0].handle;
}

void kinc_internal_call_resize_callback(int width, int height) {
	if (windows[0].resizeCallback != NULL) {
		windows[0].resizeCallback(width, height, windows[0].resizeCallbackData);
	}
}

bool kinc_internal_call_close_callback() {
	if (windows[0].closeCallback != NULL) {
		return windows[0].closeCallback(windows[0].closeCallbackData);
	}
	return true;
}
