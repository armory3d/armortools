#include "android_file_dialog.h"
#include <kinc/system.h>
#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <android_native_app_glue.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char mobile_title[1024];

ANativeActivity *kinc_android_get_activity(void);
jclass kinc_android_find_class(JNIEnv *env, const char *name);

JNIEXPORT void JNICALL Java_tech_kinc_KincActivity_onAndroidFilePicked(JNIEnv *env, jobject jobj, jstring jstr) {
	if (jstr == NULL) return;
	const char *str = (*env)->GetStringUTFChars(env, jstr, 0);
	size_t len = strlen(str);
	wchar_t filePath[len + 1];
	mbstowcs(filePath, (char *)str, len);
	filePath[len] = 0;

	kinc_internal_drop_files_callback(filePath);
	(*env)->ReleaseStringUTFChars(env, jstr, str);
}

JNIEXPORT jstring JNICALL Java_tech_kinc_KincActivity_getMobileTitle(JNIEnv *env, jobject jobj) {
	jstring result = (*env)->NewStringUTF(env, mobile_title);
	return result;
}

void AndroidFileDialogOpen() {
	ANativeActivity *activity = kinc_android_get_activity();
	JNIEnv *env;
	JavaVM *vm = kinc_android_get_activity()->vm;
	(*vm)->AttachCurrentThread(vm, &env, NULL);
	jclass kincActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	(*env)->CallStaticVoidMethod(env, kincActivityClass, (*env)->GetStaticMethodID(env, kincActivityClass, "pickFile", "()V"));
	(*vm)->DetachCurrentThread(vm);
}

wchar_t *AndroidFileDialogSave() {
	// kinc_android_get_activity()->externalDataPath; // /storage/emulated/0/Android/data/org.armorpaint/files
	mkdir("/storage/emulated/0/Pictures/ArmorPaint", 0777);
	return L"/storage/emulated/0/Pictures/ArmorPaint/untitled";
}

jstring android_permission_name(JNIEnv *env, const char *perm_name) {
	jclass ClassManifestpermission = (*env)->FindClass(env, "android/Manifest$permission");
	jfieldID lid_PERM = (*env)->GetStaticFieldID(env, ClassManifestpermission, perm_name, "Ljava/lang/String;");
	jstring ls_PERM = (jstring)((*env)->GetStaticObjectField(env, ClassManifestpermission, lid_PERM));
	return ls_PERM;
}

bool android_has_permission(struct android_app *app, const char *perm_name) {
	ANativeActivity *activity = kinc_android_get_activity();
	JNIEnv *env;
	JavaVM *vm = kinc_android_get_activity()->vm;
	(*vm)->AttachCurrentThread(vm, &env, NULL);
	bool result = false;
	jstring ls_PERM = android_permission_name(env, perm_name);
	jint PERMISSION_GRANTED = (jint)(-1);
	{
		jclass ClassPackageManager = (*env)->FindClass(env, "android/content/pm/PackageManager");
		jfieldID lid_PERMISSION_GRANTED = (*env)->GetStaticFieldID(env, ClassPackageManager, "PERMISSION_GRANTED", "I");
		PERMISSION_GRANTED = (*env)->GetStaticIntField(env, ClassPackageManager, lid_PERMISSION_GRANTED);
	}
	{
		jobject activity = app->activity->clazz;
		jclass ClassContext = (*env)->FindClass(env, "android/content/Context");
		jmethodID MethodcheckSelfPermission = (*env)->GetMethodID(env, ClassContext, "checkSelfPermission", "(Ljava/lang/String;)I");
		jint int_result = (*env)->CallIntMethod(env, activity, MethodcheckSelfPermission, ls_PERM);
		result = (int_result == PERMISSION_GRANTED);
	}
	(*vm)->DetachCurrentThread(vm);
	return result;
}

void android_request_file_permissions(struct android_app *app) {
	ANativeActivity *activity = kinc_android_get_activity();
	JNIEnv *env;
	JavaVM *vm = kinc_android_get_activity()->vm;
	(*vm)->AttachCurrentThread(vm, &env, NULL);
	jobjectArray perm_array = (*env)->NewObjectArray(env, 2, (*env)->FindClass(env, "java/lang/String"), (*env)->NewStringUTF(env, ""));
	(*env)->SetObjectArrayElement(env, perm_array, 0, android_permission_name(env, "READ_EXTERNAL_STORAGE"));
	(*env)->SetObjectArrayElement(env, perm_array, 1, android_permission_name(env, "WRITE_EXTERNAL_STORAGE"));
	jobject jactivity = app->activity->clazz;
	jclass ClassActivity = (*env)->FindClass(env, "android/app/Activity");
	jmethodID MethodrequestPermissions = (*env)->GetMethodID(env, ClassActivity, "requestPermissions", "([Ljava/lang/String;I)V");
	(*env)->CallVoidMethod(env, jactivity, MethodrequestPermissions, perm_array, 0);
	(*vm)->DetachCurrentThread(vm);
}

void android_check_permissions() {
	ANativeActivity *activity = kinc_android_get_activity();
	struct android_app *app = (struct android_app *)activity->instance;
	bool hasPermissions = android_has_permission(app, "READ_EXTERNAL_STORAGE") && android_has_permission(app, "WRITE_EXTERNAL_STORAGE");
	if (!hasPermissions) android_request_file_permissions(app);

	JNIEnv *env;
	JavaVM *vm = kinc_android_get_activity()->vm;
	(*vm)->AttachCurrentThread(vm, &env, NULL);
	jclass kincActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	JNINativeMethod methodTable[] = {
		{"onAndroidFilePicked", "(Ljava/lang/String;)V", (void *)Java_tech_kinc_KincActivity_onAndroidFilePicked},
		{"getMobileTitle", "()Ljava/lang/String;", (void *)Java_tech_kinc_KincActivity_getMobileTitle}
	};
	int methodTableSize = sizeof(methodTable) / sizeof(methodTable[0]);
	(*env)->RegisterNatives(env, kincActivityClass, methodTable, methodTableSize);
	(*vm)->DetachCurrentThread(vm);
}
