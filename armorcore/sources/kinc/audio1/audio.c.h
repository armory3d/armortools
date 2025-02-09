#include "audio.h"

#include <stdint.h>

#include <kinc/audio2/audio.h>
#include <kinc/math/core.h>
#include <kinc/threads/atomic.h>
#include <kinc/threads/mutex.h>
#include <kinc/video.h>

#include <assert.h>
#include <stdlib.h>

static kinc_mutex_t mutex;

#define CHANNEL_COUNT 16
static kinc_a1_channel_t channels[CHANNEL_COUNT];
static kinc_a1_stream_channel_t streamchannels[CHANNEL_COUNT];
static kinc_internal_video_channel_t videos[CHANNEL_COUNT];

static float sampleLinear(int16_t *data, float position) {
	int pos1 = (int)position;
	int pos2 = (int)(position + 1);
	float sample1 = data[pos1] / 32767.0f;
	float sample2 = data[pos2] / 32767.0f;
	float a = position - pos1;
	return sample1 * (1 - a) + sample2 * a;
}

static void kinc_a2_on_a1_mix(kinc_a2_buffer_t *buffer, uint32_t samples, void *userdata) {
	kinc_a1_mix(buffer, samples);
}

void kinc_a1_mix(kinc_a2_buffer_t *buffer, uint32_t samples) {
	for (uint32_t i = 0; i < samples; ++i) {
		float left_value = 0.0f;
		float right_value = 0.0f;

		kinc_mutex_lock(&mutex);
		for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (channels[i].sound != NULL) {
				left_value += sampleLinear(channels[i].sound->left, channels[i].position) * channels[i].volume * channels[i].sound->volume;
				right_value = kinc_max(kinc_min(right_value, 1.0f), -1.0f);
				right_value += sampleLinear(channels[i].sound->right, channels[i].position) * channels[i].volume * channels[i].sound->volume;
				left_value = kinc_max(kinc_min(left_value, 1.0f), -1.0f);

				channels[i].position += channels[i].pitch / channels[i].sound->sample_rate_pos;
				// channels[i].position += 2;
				if (channels[i].position + 1 >= channels[i].sound->size) {
					if (channels[i].loop) {
						channels[i].position = 0;
					}
					else {
						channels[i].sound = NULL;
					}
				}
			}
		}
		for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (streamchannels[i].stream != NULL) {
				float *samples = kinc_a1_sound_stream_next_frame(streamchannels[i].stream);
				left_value += samples[0] * kinc_a1_sound_stream_volume(streamchannels[i].stream);
				left_value = kinc_max(kinc_min(left_value, 1.0f), -1.0f);
				right_value += samples[1] * kinc_a1_sound_stream_volume(streamchannels[i].stream);
				right_value = kinc_max(kinc_min(right_value, 1.0f), -1.0f);
				if (kinc_a1_sound_stream_ended(streamchannels[i].stream)) {
					streamchannels[i].stream = NULL;
				}
			}
		}

		for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (videos[i].stream != NULL) {
				float *samples = kinc_internal_video_sound_stream_next_frame(videos[i].stream);
				left_value += samples[0];
				left_value = kinc_max(kinc_min(left_value, 1.0f), -1.0f);
				right_value += samples[1];
				right_value = kinc_max(kinc_min(right_value, 1.0f), -1.0f);
				if (kinc_internal_video_sound_stream_ended(videos[i].stream)) {
					videos[i].stream = NULL;
				}
			}
		}

		kinc_mutex_unlock(&mutex);

		assert(buffer->channel_count >= 2);
		buffer->channels[0][buffer->write_location] = left_value;
		buffer->channels[1][buffer->write_location] = right_value;

		buffer->write_location += 1;
		if (buffer->write_location >= buffer->data_size) {
			buffer->write_location = 0;
		}
	}
}

void kinc_a1_init(void) {
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		channels[i].sound = NULL;
		channels[i].position = 0;
	}
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		streamchannels[i].stream = NULL;
		streamchannels[i].position = 0;
	}
	kinc_mutex_init(&mutex);

	kinc_a2_init();
	kinc_a2_set_callback(kinc_a2_on_a1_mix, NULL);
}

kinc_a1_channel_t *kinc_a1_play_sound(kinc_a1_sound_t *sound, bool loop, float pitch, bool unique) {
	kinc_a1_channel_t *channel = NULL;
	kinc_mutex_lock(&mutex);
	bool found = false;
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (channels[i].sound == sound) {
			found = true;
			break;
		}
	}
	if (!found || !unique) {
		for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (channels[i].sound == NULL) {
				channels[i].sound = sound;
				channels[i].position = 0;
				channels[i].loop = loop;
				channels[i].pitch = pitch;
				channels[i].volume = sound->volume;
				channel = &channels[i];
				break;
			}
		}
	}
	kinc_mutex_unlock(&mutex);
	return channel;
}

void kinc_a1_stop_sound(kinc_a1_sound_t *sound) {
	kinc_mutex_lock(&mutex);
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (channels[i].sound == sound) {
			channels[i].sound = NULL;
			channels[i].position = 0;
			break;
		}
	}
	kinc_mutex_unlock(&mutex);
}

void kinc_a1_play_sound_stream(kinc_a1_sound_stream_t *stream) {
	kinc_mutex_lock(&mutex);

	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (streamchannels[i].stream == stream) {
			streamchannels[i].stream = NULL;
			streamchannels[i].position = 0;
			break;
		}
	}

	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (streamchannels[i].stream == NULL) {
			streamchannels[i].stream = stream;
			streamchannels[i].position = 0;
			break;
		}
	}

	kinc_mutex_unlock(&mutex);
}

void kinc_a1_stop_sound_stream(kinc_a1_sound_stream_t *stream) {
	kinc_mutex_lock(&mutex);
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (streamchannels[i].stream == stream) {
			streamchannels[i].stream = NULL;
			streamchannels[i].position = 0;
			break;
		}
	}
	kinc_mutex_unlock(&mutex);
}

void kinc_internal_play_video_sound_stream(struct kinc_internal_video_sound_stream *stream) {
	kinc_mutex_lock(&mutex);
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (videos[i].stream == NULL) {
			videos[i].stream = stream;
			videos[i].position = 0;
			break;
		}
	}
	kinc_mutex_unlock(&mutex);
}

void kinc_internal_stop_video_sound_stream(struct kinc_internal_video_sound_stream *stream) {
	kinc_mutex_lock(&mutex);
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (videos[i].stream == stream) {
			videos[i].stream = NULL;
			videos[i].position = 0;
			break;
		}
	}
	kinc_mutex_unlock(&mutex);
}

float kinc_a1_channel_get_volume(kinc_a1_channel_t *channel) {
	return channel->volume;
}

void kinc_a1_channel_set_volume(kinc_a1_channel_t *channel, float volume) {
	KINC_ATOMIC_EXCHANGE_FLOAT(&channel->volume, volume);
}

void kinc_a1_channel_set_pitch(kinc_a1_channel_t *channel, float pitch) {
	KINC_ATOMIC_EXCHANGE_FLOAT(&channel->pitch, pitch);
}
