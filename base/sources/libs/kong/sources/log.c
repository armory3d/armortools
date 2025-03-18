#include "log.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __android__
#include <android/log.h>
#endif

void kong_log(log_level_t level, const char *format, ...) {
	va_list args;
	va_start(args, format);
	kong_log_args(level, format, args);
	va_end(args);
}

void kong_log_args(log_level_t level, const char *format, va_list args) {
#ifdef WIN32
	{
		char buffer[4096];
		vsnprintf(buffer, 4090, format, args);
		strcat(buffer, "\r\n");
		OutputDebugStringA(buffer);
	}
#endif

	{
		char buffer[4096];
		vsnprintf(buffer, 4090, format, args);
		strcat(buffer, "\n");
		fprintf(level == LOG_LEVEL_INFO ? stdout : stderr, "%s", buffer);
	}

#ifdef __android__
	switch (level) {
	case KINC_LOG_LEVEL_INFO:
		__android_log_vprint(ANDROID_LOG_INFO, "krom", format, args);
		break;
	case KINC_LOG_LEVEL_WARNING:
		__android_log_vprint(ANDROID_LOG_WARN, "krom", format, args);
		break;
	case KINC_LOG_LEVEL_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "krom", format, args);
		break;
	}
#endif
}
