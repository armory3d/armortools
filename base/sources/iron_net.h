#pragma once

#include <iron_global.h>
#include <stdbool.h>

#define IRON_HTTPS_GET  0
#define IRON_HTTPS_POST 1

typedef void (*iron_https_callback_t)(const char *body, void *callbackdata);

void iron_net_request(const char *url_base, const char *url_path, const char *data, int port, int method, iron_https_callback_t callback, void *callbackdata, const char *dst_path);

void iron_net_update();
