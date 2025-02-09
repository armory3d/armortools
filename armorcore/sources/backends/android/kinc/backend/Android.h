#pragma once

#include <android_native_app_glue.h>

// name in usual Java syntax (points, no slashes)
jclass kinc_android_find_class(JNIEnv *env, const char *name);
ANativeActivity *kinc_android_get_activity(void);
AAssetManager *kinc_android_get_asset_manager(void);
