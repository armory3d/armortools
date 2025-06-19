// #pragma once

// #include <iron_gpu.h>

// typedef struct {
// 	void *assetReader;
// 	void *videoTrackOutput;
// 	void *audioTrackOutput;
// 	double start;
// 	double next;
// 	unsigned long long audioTime;
// 	bool playing;
// 	void *sound;
// 	void *androidVideo;
// 	int id;
// 	gpu_texture_t image;
// 	double lastTime;
// 	int myWidth;
// 	int myHeight;
// } iron_video_impl_t;

// typedef struct iron_internal_video_sound_stream {
// 	void *audioTrackOutput;
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
