#pragma once

#include <kinc/global.h>
#include <stdarg.h>

/*! \file error.h
    \brief Contains functionality to stop the program in case of an error and create a user-visible error message.

    The affirm and error functions print an error message and then exit the program. Error messages can be made
    visible to the user (unless a console window is active this is only implemented for Windows).
*/

void kinc_affirm(bool condition);
void kinc_affirm_message(bool condition, const char *format, ...);
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
#include <kinc/log.h>
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

void kinc_affirm_message(bool condition, const char *format, ...) {
	if (!condition) {
		va_list args;
		va_start(args, format);
		kinc_error_args(format, args);
		va_end(args);
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
