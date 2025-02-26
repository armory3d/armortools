#pragma once

#include <iron_global.h>
#include <stdarg.h>

typedef enum { KINC_LOG_LEVEL_INFO, KINC_LOG_LEVEL_WARNING, KINC_LOG_LEVEL_ERROR } kinc_log_level_t;
void kinc_log(kinc_log_level_t log_level, const char *format, ...);
void kinc_log_args(kinc_log_level_t log_level, const char *format, va_list args);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#ifdef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif

#ifdef KINC_WINDOWS
#include <kinc/backend/miniwindows.h>
#include <kinc/backend/system_microsoft.h>
#endif

#ifdef KINC_ANDROID
#include <android/log.h>
#endif

void kinc_log(kinc_log_level_t level, const char *format, ...) {
	va_list args;
	va_start(args, format);
	kinc_log_args(level, format, args);
	va_end(args);
}

#define UTF8

void kinc_log_args(kinc_log_level_t level, const char *format, va_list args) {
#ifdef KINC_ANDROID
	va_list args_android_copy;
	va_copy(args_android_copy, args);
	switch (level) {
	case KINC_LOG_LEVEL_INFO:
		__android_log_vprint(ANDROID_LOG_INFO, "Kinc", format, args_android_copy);
		break;
	case KINC_LOG_LEVEL_WARNING:
		__android_log_vprint(ANDROID_LOG_WARN, "Kinc", format, args_android_copy);
		break;
	case KINC_LOG_LEVEL_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "Kinc", format, args_android_copy);
		break;
	}
	va_end(args_android_copy);
#endif
#ifdef KINC_WINDOWS
#ifdef UTF8
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	wcscat(buffer, L"\r\n");
	OutputDebugStringW(buffer);
#ifdef KINC_WINDOWS
	DWORD written;
	WriteConsoleW(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)wcslen(buffer), &written, NULL);
#endif
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	strcat(buffer, "\r\n");
	OutputDebugStringA(buffer);
#ifdef KINC_WINDOWS
	DWORD written;
	WriteConsoleA(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)strlen(buffer), &written, NULL);
#endif
#endif
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	strcat(buffer, "\n");
	fprintf(level == KINC_LOG_LEVEL_INFO ? stdout : stderr, "%s", buffer);
#endif
}

#endif

void kinc_affirm(bool condition);
void kinc_affirm_args(bool condition, const char *format, va_list args);
void kinc_error(void);
void kinc_error_message(const char *format, ...);
void kinc_error_args(const char *format, va_list args);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#ifndef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <iron_log.h>
#ifndef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif
#include <stdlib.h>

#ifdef KINC_WINDOWS

#include <kinc/backend/miniwindows.h>
#include <kinc/backend/system_microsoft.h>
#endif

void kinc_affirm(bool condition) {
	if (!condition) {
		kinc_error();
	}
}

void kinc_affirm_args(bool condition, const char *format, va_list args) {
	if (!condition) {
		kinc_error_args(format, args);
	}
}

void kinc_error(void) {
	kinc_error_message("Unknown error");
}

void kinc_error_message(const char *format, ...) {
	{
		va_list args;
		va_start(args, format);
		kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KINC_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		kinc_microsoft_format(format, args, buffer);
		MessageBoxW(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif

#ifndef KINC_NO_CLIB
	exit(EXIT_FAILURE);
#endif
}

void kinc_error_args(const char *format, va_list args) {
	kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);

#ifdef KINC_WINDOWS
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	MessageBoxW(NULL, buffer, L"Error", 0);
#endif

#ifndef KINC_NO_CLIB
	exit(EXIT_FAILURE);
#endif
}

#endif
