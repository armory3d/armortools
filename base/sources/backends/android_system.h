#pragma once

#include "android_native_app_glue.h"

// name in usual Java syntax (points, no slashes)
jclass iron_android_find_class(JNIEnv *env, const char *name);
ANativeActivity *iron_android_get_activity(void);
AAssetManager *iron_android_get_asset_manager(void);
