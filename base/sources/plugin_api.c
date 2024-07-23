
#include "quickjs/quickjs.h"

extern JSRuntime *js_runtime;
extern JSContext *js_ctx;

void console_log(char *s);

static JSValue js_console_log(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
	console_log(JS_ToCString(ctx, argv[0]));
	return JS_UNDEFINED;
}

void plugin_api_init() {
    JSValue global_obj = JS_GetGlobalObject(js_ctx);
    JS_SetPropertyStr(js_ctx, global_obj, "console_log", JS_NewCFunction(js_ctx, js_console_log, "console_log", 1));
    JS_FreeValue(js_ctx, global_obj);
}
