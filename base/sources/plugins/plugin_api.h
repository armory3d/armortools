#pragma once

#include "quickjs-libc.h"
#include "quickjs.h"
#include <stdbool.h>
#include <string.h>

extern JSRuntime *js_runtime;
extern JSContext *js_ctx;
float             js_eval(const char *s);
char             *js_call(void *p);
JSValue           js_call_arg(void *p, int argc, void *argv);

// ██████╗ ███████╗███████╗██╗███╗   ██╗███████╗███████╗
// ██╔══██╗██╔════╝██╔════╝██║████╗  ██║██╔════╝██╔════╝
// ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║█████╗  ███████╗
// ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██╔══╝  ╚════██║
// ██████╔╝███████╗██║     ██║██║ ╚████║███████╗███████║
// ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚══════╝

#define FN(name) static JSValue js_##name(JSContext *ctx, JSValue this_val, int argc, JSValue *argv)

#define VOID_FN(name)        \
	void name();             \
	FN(name) {               \
		name();              \
		return JS_UNDEFINED; \
	}

#define PTR_FN(name)                         \
	void *name();                            \
	FN(name) {                               \
		uint64_t result = (uint64_t)name();  \
		return JS_NewBigUint64(ctx, result); \
	}

#define VOID_FN_STR(name)                             \
	void name(char *s);                               \
	FN(name) {                                        \
		char *s = (char *)JS_ToCString(ctx, argv[0]); \
		name(s);                                      \
		return JS_UNDEFINED;                          \
	}

#define VOID_FN_PTR_STR(name)                         \
	void name(void *p, char *s);                      \
	FN(name) {                                        \
		uint64_t p;                                   \
		JS_ToBigUint64(ctx, &p, argv[0]);             \
		char *s = (char *)JS_ToCString(ctx, argv[1]); \
		name((void *)p, s);                           \
		return JS_UNDEFINED;                          \
	}

#define VOID_FN_CB(name)                         \
	void name(void *p);                          \
	FN(name) {                                   \
		JSValue dup = JS_DupValue(ctx, argv[0]); \
		if (JS_IsNull(dup)) {                    \
			name(NULL);                          \
			return JS_UNDEFINED;                 \
		}                                        \
		JSValue *p = malloc(sizeof(JSValue));    \
		memcpy(p, &dup, sizeof(JSValue));        \
		name(p);                                 \
		return JS_UNDEFINED;                     \
	}

#define VOID_FN_PTR_CB(name)                      \
	void name(void *p0, void *p1);                \
	FN(name) {                                    \
		uint64_t p0;                              \
		JS_ToBigUint64(ctx, &p0, argv[0]);        \
		JSValue *p1  = malloc(sizeof(JSValue));   \
		JSValue  dup = JS_DupValue(ctx, argv[1]); \
		memcpy(p1, &dup, sizeof(JSValue));        \
		name((void *)p0, p1);                     \
		return JS_UNDEFINED;                      \
	}

#define BIND(name, argc) JS_SetPropertyStr(js_ctx, global_obj, #name, JS_NewCFunction(js_ctx, js_##name, #name, argc))
