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

/*float sampleHermite4pt3oX(s16* data, float position) {
    float s0 = data[(int)(position - 1)] / 32767.0f;
    float s1 = data[(int)(position + 0)] / 32767.0f;
    float s2 = data[(int)(position + 1)] / 32767.0f;
    float s3 = data[(int)(position + 2)] / 32767.0f;

    float x = position - (int)(position);

    // 4-point, 3rd-order Hermite (x-form)
    float c0 = s1;
    float c1 = 0.5f * (s2 - s0);
    float c2 = s0 - 2.5f * s1 + 2 * s2 - 0.5f * s3;
    float c3 = 0.5f * (s3 - s0) + 1.5f * (s1 - s2);
    return ((c3 * x + c2) * x + c1) * x + c0;
}*/

void kinc_a1_mix(kinc_a2_buffer_t *buffer, uint32_t samples) {
	for (uint32_t i = 0; i < samples; ++i) {
		float left_value = 0.0f;
		float right_value = 0.0f;
#if 0
		__m128 sseSamples[4];
		for (int i = 0; i < channelCount; i += 4) {
			s16 data[4];
			for (int i2 = 0; i2 < 4; ++i2) {
				if (channels[i + i2].sound != nullptr) {
					data[i2] = *(s16*)&channels[i + i2].sound->data[channels[i + i2].position];
					channels[i + i2].position += 2;
					if (channels[i + i2].position >= channels[i + i2].sound->size) channels[i + i2].sound = nullptr;
				}
				else {
					data[i2] = 0;
				}
			}
			sseSamples[i / 4] = _mm_set_ps(data[3] / 32767.0f, data[2] / 32767.0f, data[1] / 32767.0f, data[0] / 32767.0f);
		}
		__m128 a = _mm_add_ps(sseSamples[0], sseSamples[1]);
		__m128 b = _mm_add_ps(sseSamples[2], sseSamples[3]);
		__m128 c = _mm_add_ps(a, b);
		value = c.m128_f32[0] + c.m128_f32[1] + c.m128_f32[2] + c.m128_f32[3];
		value = max(min(value, 1.0f), -1.0f);
#else
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
#endif
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
