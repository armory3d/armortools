#include <kinc/video.h>

#include <kinc/audio1/audio.h>
#include <kinc/graphics4/texture.h>
#include <kinc/io/filereader.h>
#include <kinc/log.h>
#include <kinc/system.h>

#include <android_native_app_glue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <jni.h>
#include <kinc/backend/Android.h>
#include <pthread.h>

void kinc_video_sound_stream_impl_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency) {
	stream->bufferSize = 1;
	stream->bufferReadPosition = 0;
	stream->bufferWritePosition = 0;
	stream->read = 0;
	stream->written = 0;
}

void kinc_video_sound_stream_impl_destroy(kinc_internal_video_sound_stream_t *stream) {}

void kinc_video_sound_stream_impl_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count) {}

static float samples[2] = {0};

float *kinc_internal_video_sound_stream_next_frame(kinc_internal_video_sound_stream_t *stream) {
	return samples;
}

bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream) {
	return false;
}

JNIEXPORT void JNICALL Java_tech_kinc_KincMoviePlayer_nativeCreate(JNIEnv *env, jobject jobj, jstring jpath, jobject surface, jint id) {
}

void KoreAndroidVideoInit() {
	JNIEnv *env;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);

	jclass clazz = kinc_android_find_class(env, "tech.kinc.KincMoviePlayer");

	// String path, Surface surface, int id
	JNINativeMethod methodTable[] = {{"nativeCreate", "(Ljava/lang/String;Landroid/view/Surface;I)V", (void *)Java_tech_kinc_KincMoviePlayer_nativeCreate}};

	int methodTableSize = sizeof(methodTable) / sizeof(methodTable[0]);

	int failure = (*env)->RegisterNatives(env, clazz, methodTable, methodTableSize);
	if (failure != 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Failed to register KincMoviePlayer.nativeCreate");
	}

	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);
}

void kinc_video_init(kinc_video_t *video, const char *filename) {
	video->impl.playing = false;
	video->impl.sound = NULL;
}

void kinc_video_destroy(kinc_video_t *video) {
}

void kinc_video_play(kinc_video_t *video, bool loop) {
}

void kinc_video_pause(kinc_video_t *video) {
}

void kinc_video_stop(kinc_video_t *video) {
}

void kinc_video_update(kinc_video_t *video, double time) {}

int kinc_video_width(kinc_video_t *video) {
	return 512;
}

int kinc_video_height(kinc_video_t *video) {
	return 512;
}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	return NULL;
}

double kinc_video_duration(kinc_video_t *video) {
	return 0.0;
}

double kinc_video_position(kinc_video_t *video) {
	return 0.0;
}

bool kinc_video_finished(kinc_video_t *video) {
	return false;
}

bool kinc_video_paused(kinc_video_t *video) {
	return !video->impl.playing;
}
