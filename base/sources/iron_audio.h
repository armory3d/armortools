#pragma once

#ifdef IRON_A1

#include <iron_global.h>
#include <stdbool.h>

/*! \file audio.h
    \brief Audio1 is a high-level audio-API that lets you directly play audio-files. Depending on the target-system it either sits directly on a high-level
   system audio-API or is implemented based on Audio2.
*/

struct iron_internal_video_sound_stream;

struct iron_a1_channel;
typedef struct iron_a1_channel iron_a1_channel_t;

struct iron_a1_stream_channel;
typedef struct iron_a1_stream_channel iron_a1_stream_channel_t;

struct iron_internal_video_channel;
typedef struct iron_internal_video_channel iron_internal_video_channel_t;

void iron_a1_init(void);
iron_a1_channel_t *iron_a1_play_sound(iron_a1_sound_t *sound, bool loop, float pitch, bool unique);
void iron_a1_stop_sound(iron_a1_sound_t *sound);
void iron_a1_play_sound_stream(iron_a1_sound_stream_t *stream);
void iron_a1_stop_sound_stream(iron_a1_sound_stream_t *stream);
float iron_a1_channel_get_volume(iron_a1_channel_t *channel);
void iron_a1_channel_set_volume(iron_a1_channel_t *channel, float volume);
void iron_a1_channel_set_pitch(iron_a1_channel_t *channel, float pitch);
void iron_a1_mix(iron_a2_buffer_t *buffer, uint32_t samples);

void iron_internal_play_video_sound_stream(struct iron_internal_video_sound_stream *stream);
void iron_internal_stop_video_sound_stream(struct iron_internal_video_sound_stream *stream);

typedef struct iron_a1_sound {
	uint8_t channel_count;
	uint8_t bits_per_sample;
	uint32_t samples_per_second;
	int16_t *left;
	int16_t *right;
	int size;
	float sample_rate_pos;
	float volume;
	bool in_use;
} iron_a1_sound_t;

iron_a1_sound_t *iron_a1_sound_create(const char *filename);
void iron_a1_sound_destroy(iron_a1_sound_t *sound);
float iron_a1_sound_volume(iron_a1_sound_t *sound);
void iron_a1_sound_set_volume(iron_a1_sound_t *sound, float value);

struct stb_vorbis;

typedef struct iron_a1_sound_stream {
	struct stb_vorbis *vorbis;
	int chans;
	int rate;
	bool myLooping;
	float myVolume;
	bool rateDecodedHack;
	bool end;
	float samples[2];
	uint8_t *buffer;
} iron_a1_sound_stream_t;

iron_a1_sound_stream_t *iron_a1_sound_stream_create(const char *filename, bool looping);
float *iron_a1_sound_stream_next_frame(iron_a1_sound_stream_t *stream);
int iron_a1_sound_stream_channels(iron_a1_sound_stream_t *stream);
int iron_a1_sound_stream_sample_rate(iron_a1_sound_stream_t *stream);
bool iron_a1_sound_stream_looping(iron_a1_sound_stream_t *stream);
void iron_a1_sound_stream_set_looping(iron_a1_sound_stream_t *stream, bool loop);
bool iron_a1_sound_stream_ended(iron_a1_sound_stream_t *stream);
float iron_a1_sound_stream_length(iron_a1_sound_stream_t *stream);
float iron_a1_sound_stream_position(iron_a1_sound_stream_t *stream);
void iron_a1_sound_stream_reset(iron_a1_sound_stream_t *stream);
float iron_a1_sound_stream_volume(iron_a1_sound_stream_t *stream);
void iron_a1_sound_stream_set_volume(iron_a1_sound_stream_t *stream, float value);

#endif

#ifdef IRON_A2

#include <iron_global.h>

#include <stdint.h>

/*! \file audio.h
    \brief Audio2 is a low-level audio-API that allows you to directly provide a stream of audio-samples.
*/

#define IRON_A2_MAX_CHANNELS 8

typedef struct iron_a2_buffer {
	uint8_t channel_count;
	float *channels[IRON_A2_MAX_CHANNELS];
	uint32_t data_size;
	uint32_t read_location;
	uint32_t write_location;
} iron_a2_buffer_t;

void iron_a2_init(void);
void iron_a2_set_callback(void (*iron_a2_audio_callback)(iron_a2_buffer_t *buffer, uint32_t samples, void *userdata), void *userdata);
uint32_t iron_a2_samples_per_second(void);
void iron_a2_set_sample_rate_callback(void (*iron_a2_sample_rate_callback)(void *userdata), void *userdata);
void iron_a2_update(void);
void iron_a2_shutdown(void);

void iron_a2_internal_init(void);
bool iron_a2_internal_callback(iron_a2_buffer_t *buffer, int samples);
void iron_a2_internal_sample_rate_callback(void);

#ifdef IRON_IMPLEMENTATION_AUDIO2
#define IRON_IMPLEMENTATION
#endif

#ifdef IRON_IMPLEMENTATION

#include <iron_thread.h>
#include <memory.h>
#include <stddef.h>

static iron_mutex_t mutex;

static void (*a2_callback)(iron_a2_buffer_t *buffer, uint32_t samples, void *userdata) = NULL;
static void *a2_userdata = NULL;

void iron_a2_set_callback(void (*iron_a2_audio_callback)(iron_a2_buffer_t *buffer, uint32_t samples, void *userdata), void *userdata) {
	iron_mutex_lock(&mutex);
	a2_callback = iron_a2_audio_callback;
	a2_userdata = userdata;
	iron_mutex_unlock(&mutex);
}

static void (*a2_sample_rate_callback)(void *userdata) = NULL;
static void *a2_sample_rate_userdata = NULL;

void iron_a2_set_sample_rate_callback(void (*iron_a2_sample_rate_callback)(void *userdata), void *userdata) {
	iron_mutex_lock(&mutex);
	a2_sample_rate_callback = iron_a2_sample_rate_callback;
	a2_sample_rate_userdata = userdata;
	iron_mutex_unlock(&mutex);
}

void iron_a2_internal_init(void) {
	iron_mutex_init(&mutex);
}

bool iron_a2_internal_callback(iron_a2_buffer_t *buffer, int samples) {
	iron_mutex_lock(&mutex);
	bool has_callback = a2_callback != NULL;
	if (has_callback) {
		a2_callback(buffer, samples, a2_userdata);
	}
	iron_mutex_unlock(&mutex);
	return has_callback;
}

void iron_a2_internal_sample_rate_callback(void) {
	iron_mutex_lock(&mutex);
	if (a2_sample_rate_callback != NULL) {
		a2_sample_rate_callback(a2_sample_rate_userdata);
	}
	iron_mutex_unlock(&mutex);
}

#endif

#endif
