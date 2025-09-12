
#include <iron_system.h>
#include <iron_net.h>
#include <string.h>
#include <jni.h>
#include "android_native_app_glue.h"

ANativeActivity *iron_android_get_activity(void);
jclass iron_android_find_class(JNIEnv *env, const char *name);

void iron_https_request(const char *url_base, const char *url_path, const char *data, int port, int method,
                       iron_http_callback_t callback, void *callbackdata) {
    ANativeActivity *activity = iron_android_get_activity();
    JNIEnv *env;
    JavaVM *vm = iron_android_get_activity()->vm;
    (*vm)->AttachCurrentThread(vm, &env, NULL);
    jclass activityClass = iron_android_find_class(env, "arm.AndroidHttpRequest");

    jstring jurl_base = (*env)->NewStringUTF(env, url_base);
    jstring jurl_path = (*env)->NewStringUTF(env, url_path);
    jbyteArray bytes_array = (jbyteArray)((*env)->CallStaticObjectMethod(env, activityClass, (*env)->GetStaticMethodID(env, activityClass, "androidHttpRequest", "(Ljava/lang/String;Ljava/lang/String;)[B"), jurl_base, jurl_path));

    if (bytes_array == NULL) {
        callback(NULL, callbackdata);
        (*vm)->DetachCurrentThread(vm);
        return;
    }

    jsize num_bytes = (*env)->GetArrayLength(env, bytes_array);
    jbyte *elements = (*env)->GetByteArrayElements(env, bytes_array, NULL);
    if (elements != NULL) {
        callback((char *)elements, callbackdata);
        (*env)->ReleaseByteArrayElements(env, bytes_array, elements, JNI_ABORT);
    }

    (*vm)->DetachCurrentThread(vm);
}
