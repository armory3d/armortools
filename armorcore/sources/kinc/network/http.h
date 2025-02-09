#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file http.h
    \brief Provides a simple http-API.
*/

#define KINC_HTTP_GET 0
#define KINC_HTTP_POST 1
#define KINC_HTTP_PUT 2
#define KINC_HTTP_DELETE 3

typedef void (*kinc_http_callback_t)(int error, int response, const char *body, void *callbackdata);

void kinc_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                                 kinc_http_callback_t callback, void *callbackdata);

#ifdef KINC_IMPLEMENTATION_NETWORK
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#if !defined KINC_MACOS && !defined KINC_IOS && !defined KINC_WINDOWS

#include <assert.h>

void kinc_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                       kinc_http_callback_t callback, void *callbackdata) {
	assert(false); // not implemented for the current system, please send a pull-request
}

#endif

#endif
