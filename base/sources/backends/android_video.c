// #include <iron_video.h>

// #include <iron_audio.h>
// #include <iron_gpu.h>
// #include <iron_file.h>
// #include <iron_system.h>

// #include <android_native_app_glue.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>
// #include <jni.h>
// #include <backends/android.h>
// #include <pthread.h>

// void iron_video_sound_stream_impl_init(iron_internal_video_sound_stream_t *stream, int channel_count, int frequency) {
// 	stream->bufferSize = 1;
// 	stream->bufferReadPosition = 0;
// 	stream->bufferWritePosition = 0;
// 	stream->read = 0;
// 	stream->written = 0;
// }

// void iron_video_sound_stream_impl_destroy(iron_internal_video_sound_stream_t *stream) {}

// void iron_video_sound_stream_impl_insert_data(iron_internal_video_sound_stream_t *stream, float *data, int sample_count) {}

// static float samples[2] = {0};

// float *iron_internal_video_sound_stream_next_frame(iron_internal_video_sound_stream_t *stream) {
// 	return samples;
// }

// bool iron_internal_video_sound_stream_ended(iron_internal_video_sound_stream_t *stream) {
// 	return false;
// }

// JNIEXPORT void JNICALL Java_org_armory3d_IronMoviePlayer_nativeCreate(JNIEnv *env, jobject jobj, jstring jpath, jobject surface, jint id) {
// }

// void IronAndroidVideoInit() {
// 	JNIEnv *env;
// 	(*iron_android_get_activity()->vm)->AttachCurrentThread(iron_android_get_activity()->vm, &env, NULL);

// 	jclass clazz = iron_android_find_class(env, "org.armory3d.IronMoviePlayer");

// 	// String path, Surface surface, int id
// 	JNINativeMethod methodTable[] = {{"nativeCreate", "(Ljava/lang/String;Landroid/view/Surface;I)V", (void *)Java_org_armory3d_IronMoviePlayer_nativeCreate}};

// 	int methodTableSize = sizeof(methodTable) / sizeof(methodTable[0]);

// 	int failure = (*env)->RegisterNatives(env, clazz, methodTable, methodTableSize);
// 	if (failure != 0) {
// 		iron_log("Failed to register IronMoviePlayer.nativeCreate");
// 	}

// 	(*iron_android_get_activity()->vm)->DetachCurrentThread(iron_android_get_activity()->vm);
// }

// void iron_video_init(iron_video_t *video, const char *filename) {
// 	video->impl.playing = false;
// 	video->impl.sound = NULL;
// }

// void iron_video_destroy(iron_video_t *video) {
// }

// void iron_video_play(iron_video_t *video, bool loop) {
// }

// void iron_video_pause(iron_video_t *video) {
// }

// void iron_video_stop(iron_video_t *video) {
// }

// void iron_video_update(iron_video_t *video, double time) {}

// int iron_video_width(iron_video_t *video) {
// 	return 512;
// }

// int iron_video_height(iron_video_t *video) {
// 	return 512;
// }

// gpu_texture_t *iron_video_current_image(iron_video_t *video) {
// 	return NULL;
// }

// double iron_video_duration(iron_video_t *video) {
// 	return 0.0;
// }

// double iron_video_position(iron_video_t *video) {
// 	return 0.0;
// }

// bool iron_video_finished(iron_video_t *video) {
// 	return false;
// }

// bool iron_video_paused(iron_video_t *video) {
// 	return !video->impl.playing;
// }
