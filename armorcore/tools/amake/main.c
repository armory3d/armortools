
// ../../make --compile --alangjs --ccompiler gcc --cppcompiler g++
// (gcc has lto enabled)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
static JSValue js_os_exec_win(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    JSValue args = argv[0];
    JSValue val = JS_GetPropertyStr(ctx, args, "length");

    uint32_t exec_argc;
    JS_ToUint32(ctx, &exec_argc, val);

    char **exec_argv = js_mallocz(ctx, sizeof(exec_argv[0]) * (exec_argc + 1));
    for(int i = 0; i < exec_argc; i++) {
        val = JS_GetPropertyUint32(ctx, args, i);
        exec_argv[i] = JS_ToCString(ctx, val);
        JS_FreeValue(ctx, val);
    }
    exec_argv[exec_argc] = NULL;

    if (argc >= 2) {
        JSValue options = argv[1];
        val = JS_GetPropertyStr(ctx, options, "cwd");
        if (!JS_IsUndefined(val)) {
            char *cwd = JS_ToCString(ctx, val);
            JS_FreeValue(ctx, val);
            _chdir(cwd);
        }
    }

    char cmd[1024];
    cmd[0] = 0;
    for (int i = 0; i < exec_argc; ++i) {
        strcat(cmd, exec_argv[i]);
        strcat(cmd, " ");
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return JS_UNDEFINED;
}

void hlslbin(const char *from, const char *to);
JSValue js_hlslbin(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);
    hlslbin(from , to);
    return JS_UNDEFINED;
}

#endif

void alang(char *source, char *output);
static JSValue js_alang(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *source = JS_ToCString(ctx, argv[0]);
    const char *output = JS_ToCString(ctx, argv[1]);
    alang(source, output);
    return JS_UNDEFINED;
}

int ashader(char *shader_lang, char *from, char *to);
static JSValue js_ashader(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *shader_lang = JS_ToCString(ctx, argv[0]);
    const char *from = JS_ToCString(ctx, argv[1]);
    const char *to = JS_ToCString(ctx, argv[2]);
    ashader(shader_lang, from, to);
    return JS_UNDEFINED;
}

void export_k(const char *from, const char *to);
JSValue js_export_k(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);
    export_k(from, to);
    return JS_UNDEFINED;
}

void export_ico(const char *from, const char *to);
JSValue js_export_ico(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);
    export_ico(from, to);
    return JS_UNDEFINED;
}

void export_png(const char *from, const char *to, int width, int height);
JSValue js_export_png(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);
    int32_t width;
    JS_ToInt32(ctx, &width, argv[2]);
    int32_t height;
    JS_ToInt32(ctx, &height, argv[3]);
    export_png(from, to, width, height);
    return JS_UNDEFINED;
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    fseek(fp , 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    char *buffer = malloc(size + 1);
    buffer[size] = 0;
    fread(buffer, size, 1, fp);
    fclose(fp);

    JSRuntime *runtime = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(runtime);

    js_std_add_helpers(ctx, argc, argv);
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue amake = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, amake, "export_k", JS_NewCFunction(ctx, js_export_k, "export_k", 2));
    JS_SetPropertyStr(ctx, amake, "export_ico", JS_NewCFunction(ctx, js_export_ico, "export_ico", 2));
    JS_SetPropertyStr(ctx, amake, "export_png", JS_NewCFunction(ctx, js_export_png, "export_png", 4));
    #ifdef _WIN32
    JS_SetPropertyStr(ctx, amake, "os_exec_win", JS_NewCFunction(ctx, js_os_exec_win, "os_exec_win", 1));
    JS_SetPropertyStr(ctx, amake, "hlslbin", JS_NewCFunction(ctx, js_hlslbin, "hlslbin", 1));
    #endif
    JS_SetPropertyStr(ctx, amake, "alang", JS_NewCFunction(ctx, js_alang, "alang", 2));
    JS_SetPropertyStr(ctx, amake, "ashader", JS_NewCFunction(ctx, js_ashader, "ashader", 3));

    JS_SetPropertyStr(ctx, global_obj, "amake", amake);
    JS_FreeValue(ctx, global_obj);

    JSValue ret = JS_Eval(ctx, buffer, size, "make.js", JS_EVAL_TYPE_MODULE);

    if (JS_IsException(ret)) {
        js_std_dump_error(ctx);
        JS_ResetUncatchableError(ctx);
    }

    JS_RunGC(runtime);
    free(buffer);
    return 0;
}
