// Windows 7
#define WINVER 0x0601
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0601

#define NOATOM
//#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
//#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
//#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
//#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
//#define NOMSG
//#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
//#define NOSHOWWINDOW
#define NOSOUND
//#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
//#define NOUSER
//#define NOVIRTUALKEYCODES
#define NOWH
//#define NOWINMESSAGES
//#define NOWINOFFSETS
//#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Windowsx.h>

// Some types for features exclusive to later versions of Windows are copied in here.
// Use with care, make sure not to break backwards-compatibility when using them.

typedef DWORD POINTER_INPUT_TYPE;

typedef UINT32 POINTER_FLAGS;

typedef enum tagPOINTER_BUTTON_CHANGE_TYPE {
	POINTER_CHANGE_NONE,
	POINTER_CHANGE_FIRSTBUTTON_DOWN,
	POINTER_CHANGE_FIRSTBUTTON_UP,
	POINTER_CHANGE_SECONDBUTTON_DOWN,
	POINTER_CHANGE_SECONDBUTTON_UP,
	POINTER_CHANGE_THIRDBUTTON_DOWN,
	POINTER_CHANGE_THIRDBUTTON_UP,
	POINTER_CHANGE_FOURTHBUTTON_DOWN,
	POINTER_CHANGE_FOURTHBUTTON_UP,
	POINTER_CHANGE_FIFTHBUTTON_DOWN,
	POINTER_CHANGE_FIFTHBUTTON_UP,
} POINTER_BUTTON_CHANGE_TYPE;

typedef struct tagPOINTER_INFO {
	POINTER_INPUT_TYPE pointerType;
	UINT32 pointerId;
	UINT32 frameId;
	POINTER_FLAGS pointerFlags;
	HANDLE sourceDevice;
	HWND hwndTarget;
	POINT ptPixelLocation;
	POINT ptHimetricLocation;
	POINT ptPixelLocationRaw;
	POINT ptHimetricLocationRaw;
	DWORD dwTime;
	UINT32 historyCount;
	INT32 InputData;
	DWORD dwKeyStates;
	UINT64 PerformanceCount;
	POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
} POINTER_INFO;

typedef UINT32 PEN_FLAGS;

typedef UINT32 PEN_MASK;

typedef struct tagPOINTER_PEN_INFO {
	POINTER_INFO pointerInfo;
	PEN_FLAGS penFlags;
	PEN_MASK penMask;
	UINT32 pressure;
	UINT32 rotation;
	INT32 tiltX;
	INT32 tiltY;
} POINTER_PEN_INFO;

#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN 0x0246
#define WM_POINTERUP 0x0247

#define GET_POINTERID_WPARAM(wParam) (LOWORD(wParam))

enum tagPOINTER_INPUT_TYPE {
	PT_POINTER = 1,  // Generic pointer
	PT_TOUCH = 2,    // Touch
	PT_PEN = 3,      // Pen
	PT_MOUSE = 4,    // Mouse
	PT_TOUCHPAD = 5, // Touchpad
};

#include <stdio.h>

#include "windows.c.h"
#include "base.c.h"
#include "display.c.h"
#include "http.c.h"
#include "mouse.c.h"
#include "system.c.h"
#include "window.c.h"
#include "video.c.h"
