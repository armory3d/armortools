#include <stdbool.h>

struct WindowData {
	id handle;
	id view;
	bool fullscreen;
	void (*resizeCallback)(int width, int height, void *data);
	void *resizeCallbackData;
	bool (*closeCallback)(void *data);
	void *closeCallbackData;
};

static struct WindowData windows[10] = {};
static int windowCounter = 0;

#include "BasicOpenGLView.m.h"
#include "HIDGamepad.c.h"
#include "HIDManager.c.h"
#include "audio.c.h"
#include "display.m.h"
#include "mouse.m.h"
#include "system.c.h"
#include "system.m.h"
#include "window.c.h"
