#pragma once

#ifdef WITH_EVAL

#include "quickjs-libc.h"
#include "quickjs.h"

void    js_init();
float   js_eval(const char *js);
JSValue js_call_arg(void *p, int argc, JSValue *argv);
char   *js_call_ptr(void *p, void *arg);
char   *js_call_ptr_str(void *p, void *arg0, char *arg1);
void   *js_pcall_str(void *p, char *arg0);
char   *js_call(void *p);

#else

void  js_init();
float js_eval(const char *js);
void  js_call_arg(void *p, int argc, void *argv);
char *js_call_ptr(void *p, void *arg);
char *js_call_ptr_str(void *p, void *arg0, char *arg1);
void *js_pcall_str(void *p, char *arg0);
char *js_call(void *p);

#endif
