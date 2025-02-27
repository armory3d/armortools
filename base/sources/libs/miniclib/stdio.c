#include "stdio.h"

#ifdef KINC_WASM
__attribute__((import_module("imports"), import_name("js_fprintf"))) void js_fprintf(const char *format);
__attribute__((import_module("imports"), import_name("js_fopen"))) FILE *js_fopen(const char *filename);
__attribute__((import_module("imports"), import_name("js_ftell"))) long int js_ftell(FILE *stream);
__attribute__((import_module("imports"), import_name("js_fseek"))) int js_fseek(FILE *stream, long int offset, int origin);
__attribute__((import_module("imports"), import_name("js_fread"))) size_t js_fread(void *ptr, size_t size, size_t count, FILE *stream);
#endif

FILE *stdout = NULL, *stderr = NULL;

int fprintf(FILE *stream, const char *format, ...) {
#ifdef KINC_WASM
	js_fprintf(format);
#endif
	return 0;
}

int vsnprintf(char *s, size_t n, const char *format, va_list arg) {
	return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream) {
	return 0;
}

FILE *fopen(const char *filename, const char *mode) {
#ifdef KINC_WASM
	return js_fopen(filename);
#endif
	return NULL;
}

int fclose(FILE *stream) {
	return 0;
}

long int ftell(FILE *stream) {
#ifdef KINC_WASM
	return js_ftell(stream);
#endif
	return 0;
}

int fseek(FILE *stream, long int offset, int origin) {
#ifdef KINC_WASM
	return js_fseek(stream, offset, origin);
#endif
	return 0;
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream) {
#ifdef KINC_WASM
	return js_fread(ptr, size, count, stream);
#endif
	return 0;
}

int fputs(const char *str, FILE *stream) {
	return 0;
}
