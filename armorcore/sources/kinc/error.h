#pragma once

#include <kinc/global.h>

#include <stdarg.h>

/*! \file error.h
    \brief Contains functionality to stop the program in case of an error and create a user-visible error message.

    The affirm and error functions print an error message and then exit the program. Error messages can be made
    visible to the user (unless a console window is active this is only implemented for Windows).
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Exits the program when a condition is untrue and shows
/// a generic error message.
/// </summary>
/// <remarks>
/// This is an alternative to assert which also persists in release
/// builds. Use this instead of assert in situations where you want
/// your users to see what's going wrong.
/// This uses Kinc's log and error functionality to make errors
/// visible.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
void kinc_affirm(bool condition);

/// <summary>
/// Exits the program when a condition is untrue and shows
/// a provided error message.
/// </summary>
/// <remarks>
/// This is equivalent to kinc_affirm() but uses the provided message
/// instead of a generic one.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
/// <param name="format">
/// The parameter is equivalent to the first printf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second printf parameter.
/// </param>
void kinc_affirm_message(bool condition, const char *format, ...);

/// <summary>
/// Equivalent to kinc_affirm_message but uses a va_list parameter.
/// </summary>
/// <remarks>
/// You will need this if you want to provide the parameters using va_start/va_end.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
/// <param name="format">
/// The parameter is equivalent to the first vprintf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second vprintf parameter.
/// </param>
void kinc_affirm_args(bool condition, const char *format, va_list args);

/// <summary>
/// Exits the program and shows a generic error message
/// </summary>
/// <remarks>
/// Mainly this just calls exit(EXIT_FAILURE) but will also use
/// Kore's log function and on Windows show an error message box.
/// </remarks>
void kinc_error(void);

/// <summary>
/// Exits the program and shows a provided error message.
/// </summary>
/// <remarks>
/// This is equivalent to kinc_error() but uses the provided message
/// instead of a generic one.
/// </remarks>
/// <param name="format">
/// The parameter is equivalent to the first printf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second printf parameter.
/// </param>
void kinc_error_message(const char *format, ...);

/// <summary>
/// Equivalent to kinc_error_message but uses a va_list parameter.
/// </summary>
/// <remarks>
/// You will need this if you want to provide the parameters using va_start/va_end.
/// </remarks>
/// <param name="format">
/// The parameter is equivalent to the first vprintf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second vprintf parameter.
/// </param>
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

#include <kinc/backend/MiniWindows.h>
#include <kinc/backend/SystemMicrosoft.h>
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

#ifdef __cplusplus
}
#endif
