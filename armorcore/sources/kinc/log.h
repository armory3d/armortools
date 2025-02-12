#pragma once

#include <kinc/global.h>
#include <stdarg.h>

/*! \file log.h
    \brief Contains basic logging functionality.

    Logging functionality is similar to plain printf but provides some system-specific bonuses.
*/

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

#ifdef KINC_MICROSOFT
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
#ifdef KINC_MICROSOFT
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
