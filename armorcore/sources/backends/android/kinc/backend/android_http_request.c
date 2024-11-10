#include "android_http_request.h"
#include <kinc/system.h>
#include <jni.h>
#include <string.h>
#include <android_native_app_glue.h>

ANativeActivity *kinc_android_get_activity(void);
jclass kinc_android_find_class(JNIEnv *env, const char *name);

void android_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                          kinc_http_callback_t callback, void *callbackdata) {
    ANativeActivity *activity = kinc_android_get_activity();
    JNIEnv *env;
    JavaVM *vm = kinc_android_get_activity()->vm;
    (*vm)->AttachCurrentThread(vm, &env, NULL);
    jclass activityClass = kinc_android_find_class(env, "arm.AndroidHttpRequest");

    jstring jstr = (*env)->NewStringUTF(env, url);
    jbyteArray bytes_array = (jbyteArray)((*env)->CallStaticObjectMethod(env, activityClass, (*env)->GetStaticMethodID(env, activityClass, "androidHttpRequest", "(Ljava/lang/String;)[B"), jstr));
    jsize num_bytes = (*env)->GetArrayLength(env, bytes_array);
    jbyte *elements = (*env)->GetByteArrayElements(env, bytes_array, NULL);
    if (elements != NULL) {
        callback(0, 200, (char *)elements, callbackdata);
        // (*env)->ReleaseByteArrayElements(env, bytes_array, elements, JNI_ABORT);
    }

    (*vm)->DetachCurrentThread(vm);
}
