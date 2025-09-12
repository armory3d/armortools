#pragma once

#include <iron_global.h>
#include <stdbool.h>

#define IRON_HTTP_GET 0
#define IRON_HTTP_POST 1

typedef void (*iron_http_callback_t)(const char *body, void *callbackdata);

void iron_https_request(const char *url_base, const char *url_path, const char *data, int port, int method,
                        iron_http_callback_t callback, void *callbackdata);
