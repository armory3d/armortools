#pragma once

#ifdef KINC_A2

#include <kinc/global.h>

#include <stdint.h>

/*! \file audio.h
    \brief Audio2 is a low-level audio-API that allows you to directly provide a stream of audio-samples.
*/

#define KINC_A2_MAX_CHANNELS 8

typedef struct kinc_a2_buffer {
	uint8_t channel_count;
	float *channels[KINC_A2_MAX_CHANNELS];
	uint32_t data_size;
	uint32_t read_location;
	uint32_t write_location;
} kinc_a2_buffer_t;

void kinc_a2_init(void);
void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, uint32_t samples, void *userdata), void *userdata);
uint32_t kinc_a2_samples_per_second(void);
void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void *userdata), void *userdata);
void kinc_a2_update(void);
void kinc_a2_shutdown(void);

void kinc_a2_internal_init(void);
bool kinc_a2_internal_callback(kinc_a2_buffer_t *buffer, int samples);
void kinc_a2_internal_sample_rate_callback(void);

#ifdef KINC_IMPLEMENTATION_AUDIO2
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <kinc/threads/mutex.h>
#include <memory.h>
#include <stddef.h>

static kinc_mutex_t mutex;

static void (*a2_callback)(kinc_a2_buffer_t *buffer, uint32_t samples, void *userdata) = NULL;
static void *a2_userdata = NULL;

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, uint32_t samples, void *userdata), void *userdata) {
	kinc_mutex_lock(&mutex);
	a2_callback = kinc_a2_audio_callback;
	a2_userdata = userdata;
	kinc_mutex_unlock(&mutex);
}

static void (*a2_sample_rate_callback)(void *userdata) = NULL;
static void *a2_sample_rate_userdata = NULL;

void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void *userdata), void *userdata) {
	kinc_mutex_lock(&mutex);
	a2_sample_rate_callback = kinc_a2_sample_rate_callback;
	a2_sample_rate_userdata = userdata;
	kinc_mutex_unlock(&mutex);
}

void kinc_a2_internal_init(void) {
	kinc_mutex_init(&mutex);
}

bool kinc_a2_internal_callback(kinc_a2_buffer_t *buffer, int samples) {
	kinc_mutex_lock(&mutex);
	bool has_callback = a2_callback != NULL;
	if (has_callback) {
		a2_callback(buffer, samples, a2_userdata);
	}
	kinc_mutex_unlock(&mutex);
	return has_callback;
}

void kinc_a2_internal_sample_rate_callback(void) {
	kinc_mutex_lock(&mutex);
	if (a2_sample_rate_callback != NULL) {
		a2_sample_rate_callback(a2_sample_rate_userdata);
	}
	kinc_mutex_unlock(&mutex);
}

#endif

#endif
