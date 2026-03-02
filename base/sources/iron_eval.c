
#include "iron_eval.h"
#include <stdlib.h>

#ifdef WITH_EVAL
#include "quickjs-libc.h"
#include "quickjs.h"
JSRuntime *js_runtime = NULL;
JSContext *js_ctx;
#ifdef WITH_PLUGINS
void plugin_api_init();
#endif

extern int    _argc;
extern char **_argv;

void js_init() {
	js_runtime = JS_NewRuntime();
	js_ctx     = JS_NewContext(js_runtime);
	js_std_add_helpers(js_ctx, _argc, _argv);
	js_init_module_std(js_ctx, "std");
	js_init_module_os(js_ctx, "os");
#ifdef WITH_PLUGINS
	plugin_api_init();
#endif
}

// float alang_eval(char *data);

float js_eval(const char *js) {
	// return alang_eval(js);
	if (js_runtime == NULL) {
		js_init();
	}
	JSValue ret = JS_Eval(js_ctx, js, strlen(js), "iron", JS_EVAL_TYPE_GLOBAL);
	if (JS_IsException(ret)) {
		js_std_dump_error(js_ctx);
		JS_ResetUncatchableError(js_ctx);
	}
	double d;
	JS_ToFloat64(js_ctx, &d, ret);
	JS_RunGC(js_runtime);
	if (d != d) { // nan
		d = 0.0;
	}
	return d;
}

JSValue js_call_arg(void *p, int argc, JSValue *argv) {
	if (js_runtime == NULL) {
		js_init();
	}
	JSValue fn             = *(JSValue *)p;
	JSValue global_obj     = JS_GetGlobalObject(js_ctx);
	JSValue js_call_result = JS_Call(js_ctx, fn, global_obj, argc, argv);
	if (JS_IsException(js_call_result)) {
		js_std_dump_error(js_ctx);
		JS_ResetUncatchableError(js_ctx);
	}
	JS_FreeValue(js_ctx, global_obj);
	return js_call_result;
}

char *js_call_ptr(void *p, void *arg) {
	JSValue argv[] = {JS_NewBigUint64(js_ctx, (uint64_t)arg)};
	return (char *)JS_ToCString(js_ctx, js_call_arg(p, 1, argv));
}

char *js_call_ptr_str(void *p, void *arg0, char *arg1) {
	JSValue argv[] = {JS_NewBigUint64(js_ctx, (uint64_t)arg0), JS_NewString(js_ctx, arg1)};
	return (char *)JS_ToCString(js_ctx, js_call_arg(p, 2, argv));
}

void *js_pcall_str(void *p, char *arg0) {
	JSValue  argv[] = {JS_NewString(js_ctx, arg0)};
	uint64_t result;
	JS_ToBigUint64(js_ctx, &result, js_call_arg(p, 1, argv));
	return (void *)result;
}

char *js_call(void *p) {
	return (char *)JS_ToCString(js_ctx, js_call_arg(p, 0, NULL));
}

#else

void js_init() {}

float js_eval(const char *js) {
	return 0.0;
}

void js_call_arg(void *p, int argc, void *argv) {}

char *js_call_ptr(void *p, void *arg) {
	return NULL;
}

char *js_call_ptr_str(void *p, void *arg0, char *arg1) {
	return NULL;
}

void *js_pcall_str(void *p, char *arg0) {
	return NULL;
}

char *js_call(void *p) {
	return NULL;
}

#endif
