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
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
#include <OMXAL/OpenMAXAL.h>
#include <OMXAL/OpenMAXAL_Android.h>
#endif
#include <assert.h>
#include <jni.h>
#include <kinc/backend/Android.h>
#include <pthread.h>
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#endif

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

#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)

#define videosCount 10
static kinc_video_t *videos[videosCount] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#define NB_MAXAL_INTERFACES 3 // XAAndroidBufferQueueItf, XAStreamInformationItf and XAPlayItf
#define NB_BUFFERS 8
#define MPEG2_TS_PACKET_SIZE 188
#define PACKETS_PER_BUFFER 10
#define BUFFER_SIZE (PACKETS_PER_BUFFER * MPEG2_TS_PACKET_SIZE)
static const int kEosBufferCntxt = 1980; // a magic value we can compare against

typedef struct kinc_android_video {
	XAObjectItf engineObject;
	XAEngineItf engineEngine;
	XAObjectItf outputMixObject;
	const char *path;

	AAsset *file;
	XAObjectItf playerObj;
	XAPlayItf playerPlayItf;
	XAAndroidBufferQueueItf playerBQItf;
	XAStreamInformationItf playerStreamInfoItf;
	XAVolumeItf playerVolItf;
	char dataCache[BUFFER_SIZE * NB_BUFFERS];
	ANativeWindow *theNativeWindow;
	jboolean reachedEof;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool discontinuity;
} kinc_android_video_t;

void kinc_android_video_init(kinc_android_video_t *video) {
	video->engineObject = NULL;
	video->engineEngine = NULL;
	video->outputMixObject = NULL;
	video->file = NULL;
	video->playerObj = NULL;
	video->playerPlayItf = NULL;
	video->playerBQItf = NULL;
	video->playerStreamInfoItf = NULL;
	video->playerVolItf = NULL;
	video->theNativeWindow = NULL;
	video->reachedEof = JNI_FALSE;
	memset(&video->mutex, 0, sizeof(video->mutex)); // mutex = PTHREAD_MUTEX_INITIALIZER; // simple assign stopped working in Android Studio 2.2
	memset(&video->cond, 0, sizeof(video->cond));   // cond = PTHREAD_COND_INITIALIZER; // simple assign stopped working in Android Studio 2.2
	video->discontinuity = false;
}

bool kinc_android_video_enqueue_initial_buffers(kinc_android_video_t *video, bool discontinuity) {
	// Fill our cache.
	// We want to read whole packets (integral multiples of MPEG2_TS_PACKET_SIZE).
	// fread returns units of "elements" not bytes, so we ask for 1-byte elements
	// and then check that the number of elements is a multiple of the packet size.
	//
	size_t bytesRead;
	// bytesRead = fread(dataCache, 1, BUFFER_SIZE * NB_BUFFERS, file);
	bytesRead = AAsset_read(video->file, video->dataCache, BUFFER_SIZE * NB_BUFFERS);
	if (bytesRead <= 0) {
		// could be premature EOF or I/O error
		return false;
	}
	if ((bytesRead % MPEG2_TS_PACKET_SIZE) != 0) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Dropping last packet because it is not whole");
	}
	size_t packetsRead = bytesRead / MPEG2_TS_PACKET_SIZE;
	kinc_log(KINC_LOG_LEVEL_INFO, "Initially queueing %zu packets", packetsRead);

	// Enqueue the content of our cache before starting to play,
	// we don't want to starve the player
	size_t i;
	for (i = 0; i < NB_BUFFERS && packetsRead > 0; i++) {
		// compute size of this buffer
		size_t packetsThisBuffer = packetsRead;
		if (packetsThisBuffer > PACKETS_PER_BUFFER) {
			packetsThisBuffer = PACKETS_PER_BUFFER;
		}
		size_t bufferSize = packetsThisBuffer * MPEG2_TS_PACKET_SIZE;
		XAresult res;
		if (discontinuity) {
			// signal discontinuity
			XAAndroidBufferItem items[1];
			items[0].itemKey = XA_ANDROID_ITEMKEY_DISCONTINUITY;
			items[0].itemSize = 0;
			// DISCONTINUITY message has no parameters,
			//   so the total size of the message is the size of the key
			//   plus the size if itemSize, both XAuint32
			res = (*video->playerBQItf)
			          ->Enqueue(video->playerBQItf, NULL /*pBufferContext*/, video->dataCache + i * BUFFER_SIZE, bufferSize, items /*pMsg*/,
			                    sizeof(XAuint32) * 2 /*msgLength*/);
			discontinuity = JNI_FALSE;
		}
		else {
			res = (*video->playerBQItf)->Enqueue(video->playerBQItf, NULL /*pBufferContext*/, video->dataCache + i * BUFFER_SIZE, bufferSize, NULL, 0);
		}
		assert(XA_RESULT_SUCCESS == res);
		packetsRead -= packetsThisBuffer;
	}

	return true;
}

static XAresult AndroidBufferQueueCallback(XAAndroidBufferQueueItf caller, void *pCallbackContext, /* input */
                                           void *pBufferContext,                                   /* input */
                                           void *pBufferData,                                      /* input */
                                           XAuint32 dataSize,                                      /* input */
                                           XAuint32 dataUsed,                                      /* input */
                                           const XAAndroidBufferItem *pItems,                      /* input */
                                           XAuint32 itemsLength /* input */) {
	kinc_android_video_t *self = (kinc_android_video_t *)pCallbackContext;
	XAresult res;
	int ok;

	// pCallbackContext was specified as NULL at RegisterCallback and is unused here
	// assert(NULL == pCallbackContext);

	// note there is never any contention on this mutex unless a discontinuity request is active
	ok = pthread_mutex_lock(&self->mutex);
	assert(0 == ok);

	// was a discontinuity requested?
	if (self->discontinuity) {
		// Note: can't rewind after EOS, which we send when reaching EOF
		// (don't send EOS if you plan to play more content through the same player)
		if (!self->reachedEof) {
			// clear the buffer queue
			res = (*self->playerBQItf)->Clear(self->playerBQItf);
			assert(XA_RESULT_SUCCESS == res);
			// rewind the data source so we are guaranteed to be at an appropriate point
			// rewind(file);
			AAsset_seek(self->file, 0, SEEK_SET);
			// Enqueue the initial buffers, with a discontinuity indicator on first buffer
			kinc_android_video_enqueue_initial_buffers(self, JNI_TRUE);
		}
		// acknowledge the discontinuity request
		self->discontinuity = JNI_FALSE;
		ok = pthread_cond_signal(&self->cond);
		assert(0 == ok);
		goto exit;
	}

	if ((pBufferData == NULL) && (pBufferContext != NULL)) {
		const int processedCommand = *(int *)pBufferContext;
		if (kEosBufferCntxt == processedCommand) {
			kinc_log(KINC_LOG_LEVEL_INFO, "EOS was processed");
			// our buffer with the EOS message has been consumed
			assert(0 == dataSize);
			goto exit;
		}
	}

	// pBufferData is a pointer to a buffer that we previously Enqueued
	assert((dataSize > 0) && ((dataSize % MPEG2_TS_PACKET_SIZE) == 0));
	assert(self->dataCache <= (char *)pBufferData && (char *)pBufferData < &self->dataCache[BUFFER_SIZE * NB_BUFFERS]);
	assert(0 == (((char *)pBufferData - self->dataCache) % BUFFER_SIZE));

	// don't bother trying to read more data once we've hit EOF
	if (self->reachedEof) {
		goto exit;
	}

	size_t nbRead;
	// note we do call fread from multiple threads, but never concurrently
	size_t bytesRead;
	// bytesRead = fread(pBufferData, 1, BUFFER_SIZE, file);
	bytesRead = AAsset_read(self->file, pBufferData, BUFFER_SIZE);
	if (bytesRead > 0) {
		if ((bytesRead % MPEG2_TS_PACKET_SIZE) != 0) {
			kinc_log(KINC_LOG_LEVEL_INFO, "Dropping last packet because it is not whole");
		}
		size_t packetsRead = bytesRead / MPEG2_TS_PACKET_SIZE;
		size_t bufferSize = packetsRead * MPEG2_TS_PACKET_SIZE;
		res = (*caller)->Enqueue(caller, NULL /*pBufferContext*/, pBufferData /*pData*/, bufferSize /*dataLength*/, NULL /*pMsg*/, 0 /*msgLength*/);
		assert(XA_RESULT_SUCCESS == res);
	}
	else {
		// EOF or I/O error, signal EOS
		XAAndroidBufferItem msgEos[1];
		msgEos[0].itemKey = XA_ANDROID_ITEMKEY_EOS;
		msgEos[0].itemSize = 0;
		// EOS message has no parameters, so the total size of the message is the size of the key
		//   plus the size if itemSize, both XAuint32
		res = (*caller)->Enqueue(caller, (void *)&kEosBufferCntxt /*pBufferContext*/, NULL /*pData*/, 0 /*dataLength*/, msgEos /*pMsg*/,
		                         sizeof(XAuint32) * 2 /*msgLength*/);
		assert(XA_RESULT_SUCCESS == res);
		self->reachedEof = JNI_TRUE;
	}

exit:
	ok = pthread_mutex_unlock(&self->mutex);
	assert(0 == ok);
	return XA_RESULT_SUCCESS;
}

static void StreamChangeCallback(XAStreamInformationItf caller, XAuint32 eventId, XAuint32 streamIndex, void *pEventData, void *pContext) {
	kinc_log(KINC_LOG_LEVEL_INFO, "StreamChangeCallback called for stream %u", streamIndex);
	kinc_android_video_t *self = (kinc_android_video_t *)pContext;
	// pContext was specified as NULL at RegisterStreamChangeCallback and is unused here
	// assert(NULL == pContext);
	switch (eventId) {
	case XA_STREAMCBEVENT_PROPERTYCHANGE: {
		// From spec 1.0.1:
		//  "This event indicates that stream property change has occurred.
		//  The streamIndex parameter identifies the stream with the property change.
		//  The pEventData parameter for this event is not used and shall be ignored."
		//

		XAresult res;
		XAuint32 domain;
		res = (*caller)->QueryStreamType(caller, streamIndex, &domain);
		assert(XA_RESULT_SUCCESS == res);
		switch (domain) {
		case XA_DOMAINTYPE_VIDEO: {
			XAVideoStreamInformation videoInfo;
			res = (*caller)->QueryStreamInformation(caller, streamIndex, &videoInfo);
			assert(XA_RESULT_SUCCESS == res);
			kinc_log(KINC_LOG_LEVEL_INFO, "Found video size %u x %u, codec ID=%u, frameRate=%u, bitRate=%u, duration=%u ms", videoInfo.width, videoInfo.height,
			         videoInfo.codecId, videoInfo.frameRate, videoInfo.bitRate, videoInfo.duration);
		} break;
		default:
			kinc_log(KINC_LOG_LEVEL_ERROR, "Unexpected domain %u\n", domain);
			break;
		}
	} break;
	default:
		kinc_log(KINC_LOG_LEVEL_ERROR, "Unexpected stream event ID %u\n", eventId);
		break;
	}
}

bool kinc_android_video_open(kinc_android_video_t *video, const char *filename) {
	XAresult res;

	// create engine
	res = xaCreateEngine(&video->engineObject, 0, NULL, 0, NULL, NULL);
	assert(XA_RESULT_SUCCESS == res);

	// realize the engine
	res = (*video->engineObject)->Realize(video->engineObject, XA_BOOLEAN_FALSE);
	assert(XA_RESULT_SUCCESS == res);

	// get the engine interface, which is needed in order to create other objects
	res = (*video->engineObject)->GetInterface(video->engineObject, XA_IID_ENGINE, &video->engineEngine);
	assert(XA_RESULT_SUCCESS == res);

	// create output mix
	res = (*video->engineEngine)->CreateOutputMix(video->engineEngine, &video->outputMixObject, 0, NULL, NULL);
	assert(XA_RESULT_SUCCESS == res);

	// realize the output mix
	res = (*video->outputMixObject)->Realize(video->outputMixObject, XA_BOOLEAN_FALSE);
	assert(XA_RESULT_SUCCESS == res);

	// open the file to play
	video->file = AAssetManager_open(kinc_android_get_asset_manager(), filename, AASSET_MODE_STREAMING);
	if (video->file == NULL) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Could not find video file.");
		return false;
	}

	// configure data source
	XADataLocator_AndroidBufferQueue loc_abq = {XA_DATALOCATOR_ANDROIDBUFFERQUEUE, NB_BUFFERS};
	XADataFormat_MIME format_mime = {XA_DATAFORMAT_MIME, XA_ANDROID_MIME_MP2TS, XA_CONTAINERTYPE_MPEG_TS};
	XADataSource dataSrc = {&loc_abq, &format_mime};

	// configure audio sink
	XADataLocator_OutputMix loc_outmix = {XA_DATALOCATOR_OUTPUTMIX, video->outputMixObject};
	XADataSink audioSnk = {&loc_outmix, NULL};

	// configure image video sink
	XADataLocator_NativeDisplay loc_nd = {
	    XA_DATALOCATOR_NATIVEDISPLAY, // locatorType
	    // the video sink must be an ANativeWindow created from a Surface or SurfaceTexture
	    (void *)video->theNativeWindow, // hWindow
	    // must be NULL
	    NULL // hDisplay
	};
	XADataSink imageVideoSink = {&loc_nd, NULL};

	// declare interfaces to use
	XAboolean required[NB_MAXAL_INTERFACES] = {XA_BOOLEAN_TRUE, XA_BOOLEAN_TRUE, XA_BOOLEAN_TRUE};
	XAInterfaceID iidArray[NB_MAXAL_INTERFACES] = {XA_IID_PLAY, XA_IID_ANDROIDBUFFERQUEUESOURCE, XA_IID_STREAMINFORMATION};

	// create media player
	res = (*video->engineEngine)
	          ->CreateMediaPlayer(video->engineEngine, &video->playerObj, &dataSrc, NULL, &audioSnk, &imageVideoSink, NULL, NULL,
	                              NB_MAXAL_INTERFACES /*XAuint32 numInterfaces*/, iidArray /*const XAInterfaceID *pInterfaceIds*/,
	                              required /*const XAboolean *pInterfaceRequired*/);
	assert(XA_RESULT_SUCCESS == res);

	// realize the player
	res = (*video->playerObj)->Realize(video->playerObj, XA_BOOLEAN_FALSE);
	assert(XA_RESULT_SUCCESS == res);

	// get the play interface
	res = (*video->playerObj)->GetInterface(video->playerObj, XA_IID_PLAY, &video->playerPlayItf);
	assert(XA_RESULT_SUCCESS == res);

	// get the stream information interface (for video size)
	res = (*video->playerObj)->GetInterface(video->playerObj, XA_IID_STREAMINFORMATION, &video->playerStreamInfoItf);
	assert(XA_RESULT_SUCCESS == res);

	// get the volume interface
	res = (*video->playerObj)->GetInterface(video->playerObj, XA_IID_VOLUME, &video->playerVolItf);
	assert(XA_RESULT_SUCCESS == res);

	// get the Android buffer queue interface
	res = (*video->playerObj)->GetInterface(video->playerObj, XA_IID_ANDROIDBUFFERQUEUESOURCE, &video->playerBQItf);
	assert(XA_RESULT_SUCCESS == res);

	// specify which events we want to be notified of
	res = (*video->playerBQItf)->SetCallbackEventsMask(video->playerBQItf, XA_ANDROIDBUFFERQUEUEEVENT_PROCESSED);
	assert(XA_RESULT_SUCCESS == res);

	// register the callback from which OpenMAX AL can retrieve the data to play
	res = (*video->playerBQItf)->RegisterCallback(video->playerBQItf, AndroidBufferQueueCallback, video);
	assert(XA_RESULT_SUCCESS == res);

	// we want to be notified of the video size once it's found, so we register a callback for that
	res = (*video->playerStreamInfoItf)->RegisterStreamChangeCallback(video->playerStreamInfoItf, StreamChangeCallback, video);
	assert(XA_RESULT_SUCCESS == res);

	// enqueue the initial buffers
	if (!kinc_android_video_enqueue_initial_buffers(video, false)) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Could not enqueue initial buffers for video decoding.");
		return false;
	}

	// prepare the player
	res = (*video->playerPlayItf)->SetPlayState(video->playerPlayItf, XA_PLAYSTATE_PAUSED);
	assert(XA_RESULT_SUCCESS == res);

	// set the volume
	res = (*video->playerVolItf)->SetVolumeLevel(video->playerVolItf, 0);
	assert(XA_RESULT_SUCCESS == res);

	// start the playback
	res = (*video->playerPlayItf)->SetPlayState(video->playerPlayItf, XA_PLAYSTATE_PLAYING);
	assert(XA_RESULT_SUCCESS == res);

	kinc_log(KINC_LOG_LEVEL_INFO, "Successfully loaded video.");

	return true;
}

void kinc_android_video_shutdown(kinc_android_video_t *video) {
	// destroy streaming media player object, and invalidate all associated interfaces
	if (video->playerObj != NULL) {
		(*video->playerObj)->Destroy(video->playerObj);
		video->playerObj = NULL;
		video->playerPlayItf = NULL;
		video->playerBQItf = NULL;
		video->playerStreamInfoItf = NULL;
		video->playerVolItf = NULL;
	}

	// destroy output mix object, and invalidate all associated interfaces
	if (video->outputMixObject != NULL) {
		(*video->outputMixObject)->Destroy(video->outputMixObject);
		video->outputMixObject = NULL;
	}

	// destroy engine object, and invalidate all associated interfaces
	if (video->engineObject != NULL) {
		(*video->engineObject)->Destroy(video->engineObject);
		video->engineObject = NULL;
		video->engineEngine = NULL;
	}

	// close the file
	if (video->file != NULL) {
		AAsset_close(video->file);
		video->file = NULL;
	}

	// make sure we don't leak native windows
	if (video->theNativeWindow != NULL) {
		ANativeWindow_release(video->theNativeWindow);
		video->theNativeWindow = NULL;
	}
}

#endif

JNIEXPORT void JNICALL Java_tech_kinc_KincMoviePlayer_nativeCreate(JNIEnv *env, jobject jobj, jstring jpath, jobject surface, jint id) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
	kinc_android_video_t *av = malloc(sizeof *av);
	kinc_android_video_init(av);
	av->theNativeWindow = ANativeWindow_fromSurface(env, surface);
	kinc_android_video_open(av, path);
	for (int i = 0; i < 10; ++i) {
		if (videos[i] != NULL && videos[i]->impl.id == id) {
			videos[i]->impl.androidVideo = av;
			break;
		}
	}
	(*env)->ReleaseStringUTFChars(env, jpath, path);
#endif
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
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	kinc_log(KINC_LOG_LEVEL_INFO, "Opening video %s.", filename);
	video->impl.myWidth = 1023;
	video->impl.myHeight = 684;

	video->impl.next = 0;
	video->impl.audioTime = 0;

	JNIEnv *env = NULL;
	(*kinc_android_get_activity()->vm)->AttachCurrentThread(kinc_android_get_activity()->vm, &env, NULL);
	jclass koreMoviePlayerClass = kinc_android_find_class(env, "tech.kinc.KincMoviePlayer");
	jmethodID constructor = (*env)->GetMethodID(env, koreMoviePlayerClass, "<init>", "(Ljava/lang/String;)V");
	jobject object = (*env)->NewObject(env, koreMoviePlayerClass, constructor, (*env)->NewStringUTF(env, filename));

	jmethodID getId = (*env)->GetMethodID(env, koreMoviePlayerClass, "getId", "()I");
	video->impl.id = (*env)->CallIntMethod(env, object, getId);

	for (int i = 0; i < videosCount; ++i) {
		if (videos[i] == NULL) {
			videos[i] = video;
			break;
		}
	}

	jmethodID jinit = (*env)->GetMethodID(env, koreMoviePlayerClass, "init", "()V");
	(*env)->CallVoidMethod(env, object, jinit);

	jmethodID getTextureId = (*env)->GetMethodID(env, koreMoviePlayerClass, "getTextureId", "()I");
	int texid = (*env)->CallIntMethod(env, object, getTextureId);

	(*kinc_android_get_activity()->vm)->DetachCurrentThread(kinc_android_get_activity()->vm);

	kinc_g4_texture_init_from_id(&video->impl.image, texid);
#endif
}

void kinc_video_destroy(kinc_video_t *video) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	kinc_video_stop(video);
	kinc_android_video_t *av = (kinc_android_video_t *)video->impl.androidVideo;
	kinc_android_video_shutdown(av);
	for (int i = 0; i < 10; ++i) {
		if (videos[i] == video) {
			videos[i] = NULL;
			break;
		}
	}
#endif
}

void kinc_video_play(kinc_video_t *video, bool loop) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	video->impl.playing = true;
	video->impl.start = kinc_time();
#endif
}

void kinc_video_pause(kinc_video_t *video) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	video->impl.playing = false;
#endif
}

void kinc_video_stop(kinc_video_t *video) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	kinc_video_pause(video);
#endif
}

void kinc_video_update(kinc_video_t *video, double time) {}

int kinc_video_width(kinc_video_t *video) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	return video->impl.myWidth;
#else
	return 512;
#endif
}

int kinc_video_height(kinc_video_t *video) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	return video->impl.myHeight;
#else
	return 512;
#endif
}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
#if KINC_ANDROID_API >= 15 && !defined(KINC_VULKAN)
	return &video->impl.image;
#else
	return NULL;
#endif
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
