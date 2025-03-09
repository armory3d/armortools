#pragma once

#include <stdbool.h>

typedef void (*iron_http_callback_t)(int error, int response, const char *body, void *callbackdata);

void android_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
						  iron_http_callback_t callback, void *callbackdata);

