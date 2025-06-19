// #pragma once

// #include <objc/runtime.h>
// #include <iron_gpu.h>

// typedef struct {
// 	double start;
// 	double videoStart;
// 	double next;
// 	unsigned long long audioTime;
// 	bool playing;
// 	bool loop;
// 	void *sound;
// 	bool image_initialized;
// 	gpu_texture_t image;
// 	double lastTime;
// 	float duration;
// 	bool finished;
// 	int myWidth;
// 	int myHeight;

// 	id videoAsset;
// 	id assetReader;
// 	id videoTrackOutput;
// 	id audioTrackOutput;
// 	id url;
// } iron_video_impl_t;

// typedef struct iron_internal_video_sound_stream {
// 	float *buffer;
// 	int bufferSize;
// 	int bufferWritePosition;
// 	int bufferReadPosition;
// 	uint64_t read;
// 	uint64_t written;
// } iron_internal_video_sound_stream_t;

// void iron_internal_video_sound_stream_init(iron_internal_video_sound_stream_t *stream, int channel_count, int frequency);
// void iron_internal_video_sound_stream_destroy(iron_internal_video_sound_stream_t *stream);
// void iron_internal_video_sound_stream_insert_data(iron_internal_video_sound_stream_t *stream, float *data, int sample_count);
// float *iron_internal_video_sound_stream_next_frame(iron_internal_video_sound_stream_t *stream);
// bool iron_internal_video_sound_stream_ended(iron_internal_video_sound_stream_t *stream);
