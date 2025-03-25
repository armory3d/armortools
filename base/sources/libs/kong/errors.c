#include "errors.h"

#include "log.h"

#include <stdlib.h>
#include <string.h>

void error_args(debug_context context, const char *message, va_list args) {
	char buffer[4096];

	if (context.filename != NULL) {
		sprintf(buffer, "In column %i at line %i in %s: ", context.column + 1, context.line + 1, context.filename);
	}
	else {
		sprintf(buffer, "In column %i at line %i: ", context.column + 1, context.line + 1);
	}

	strcat(buffer, message);

	kong_log_args(LOG_LEVEL_ERROR, buffer, args);

	exit(1);
}

void error_args_no_context(const char *message, va_list args) {
	kong_log_args(LOG_LEVEL_ERROR, message, args);

	exit(1);
}

void error(debug_context context, const char *message, ...) {
	va_list args;
	va_start(args, message);
	error_args(context, message, args);
	va_end(args);
}

void error_no_context(const char *message, ...) {
	va_list args;
	va_start(args, message);
	error_args_no_context(message, args);
	va_end(args);
}

void check_args(bool test, debug_context context, const char *message, va_list args) {
	if (!test) {
		error_args(context, message, args);
	}
}

void check_function(bool test, debug_context context, const char *message, ...) {
	if (!test) {
		va_list args;
		va_start(args, message);
		error_args(context, message, args);
		va_end(args);
	}
}
