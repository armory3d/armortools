#pragma once

#include <objc/runtime.h>
#include <kinc/graphics4/texture.h>

typedef struct {
	double start;
	double videoStart;
	double next;
	unsigned long long audioTime;
	bool playing;
	bool loop;
	void *sound;
	bool image_initialized;
	kinc_g4_texture_t image;
	double lastTime;
	float duration;
	bool finished;
	int myWidth;
	int myHeight;

	id videoAsset;
	id assetReader;
	id videoTrackOutput;
	id audioTrackOutput;
	id url;
} kinc_video_impl_t;

typedef struct kinc_internal_video_sound_stream {
	float *buffer;
	int bufferSize;
	int bufferWritePosition;
	int bufferReadPosition;
	uint64_t read;
	uint64_t written;
} kinc_internal_video_sound_stream_t;

void kinc_internal_video_sound_stream_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency);
void kinc_internal_video_sound_stream_destroy(kinc_internal_video_sound_stream_t *stream);
void kinc_internal_video_sound_stream_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count);
float *kinc_internal_video_sound_stream_next_frame(kinc_internal_video_sound_stream_t *stream);
bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream);
