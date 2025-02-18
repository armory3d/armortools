#pragma once

#ifdef KINC_A1

#include <kinc/global.h>
#include <stdbool.h>

/*! \file audio.h
    \brief Audio1 is a high-level audio-API that lets you directly play audio-files. Depending on the target-system it either sits directly on a high-level
   system audio-API or is implemented based on Audio2.
*/

struct kinc_internal_video_sound_stream;

struct kinc_a1_channel;
typedef struct kinc_a1_channel kinc_a1_channel_t;

struct kinc_a1_stream_channel;
typedef struct kinc_a1_stream_channel kinc_a1_stream_channel_t;

struct kinc_internal_video_channel;
typedef struct kinc_internal_video_channel kinc_internal_video_channel_t;

void kinc_a1_init(void);
kinc_a1_channel_t *kinc_a1_play_sound(kinc_a1_sound_t *sound, bool loop, float pitch, bool unique);
void kinc_a1_stop_sound(kinc_a1_sound_t *sound);
void kinc_a1_play_sound_stream(kinc_a1_sound_stream_t *stream);
void kinc_a1_stop_sound_stream(kinc_a1_sound_stream_t *stream);
float kinc_a1_channel_get_volume(kinc_a1_channel_t *channel);
void kinc_a1_channel_set_volume(kinc_a1_channel_t *channel, float volume);
void kinc_a1_channel_set_pitch(kinc_a1_channel_t *channel, float pitch);
void kinc_a1_mix(kinc_a2_buffer_t *buffer, uint32_t samples);

void kinc_internal_play_video_sound_stream(struct kinc_internal_video_sound_stream *stream);
void kinc_internal_stop_video_sound_stream(struct kinc_internal_video_sound_stream *stream);

typedef struct kinc_a1_sound {
	uint8_t channel_count;
	uint8_t bits_per_sample;
	uint32_t samples_per_second;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float volume;
	bool in_use;
} kinc_a1_sound_t;

kinc_a1_sound_t *kinc_a1_sound_create(const char *filename);
void kinc_a1_sound_destroy(kinc_a1_sound_t *sound);
float kinc_a1_sound_volume(kinc_a1_sound_t *sound);
void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value);

struct stb_vorbis;

typedef struct kinc_a1_sound_stream {
	struct stb_vorbis *vorbis;
	int chans;
	int rate;
	bool myLooping;
	float myVolume;
	bool rateDecodedHack;
	bool end;
	float samples[2];
	uint8_t *buffer;
} kinc_a1_sound_stream_t;

kinc_a1_sound_stream_t *kinc_a1_sound_stream_create(const char *filename, bool looping);
float *kinc_a1_sound_stream_next_frame(kinc_a1_sound_stream_t *stream);
int kinc_a1_sound_stream_channels(kinc_a1_sound_stream_t *stream);
int kinc_a1_sound_stream_sample_rate(kinc_a1_sound_stream_t *stream);
bool kinc_a1_sound_stream_looping(kinc_a1_sound_stream_t *stream);
void kinc_a1_sound_stream_set_looping(kinc_a1_sound_stream_t *stream, bool loop);
bool kinc_a1_sound_stream_ended(kinc_a1_sound_stream_t *stream);
float kinc_a1_sound_stream_length(kinc_a1_sound_stream_t *stream);
float kinc_a1_sound_stream_position(kinc_a1_sound_stream_t *stream);
void kinc_a1_sound_stream_reset(kinc_a1_sound_stream_t *stream);
float kinc_a1_sound_stream_volume(kinc_a1_sound_stream_t *stream);
void kinc_a1_sound_stream_set_volume(kinc_a1_sound_stream_t *stream, float value);

#endif
