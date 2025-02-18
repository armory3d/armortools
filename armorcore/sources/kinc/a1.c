#include "a1.h"

#ifdef KINC_A1

#include <kinc/a2.h>
#include <kinc/core.h>
#include <kinc/atomic.h>
#include <kinc/mutex.h>
#include <kinc/video.h>

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

struct kinc_a1_channel {
	kinc_a1_sound_t *sound;
	float position;
	bool loop;
	volatile float volume;
	volatile float pitch;
};

struct kinc_a1_stream_channel {
	kinc_a1_sound_stream_t *stream;
	int position;
};

struct kinc_internal_video_channel {
	struct kinc_internal_video_sound_stream *stream;
	int position;
};

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

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

struct WaveData {
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t bytesPerSecond;
	uint16_t bitsPerSample;
	uint32_t dataSize;
	uint8_t *data;
};

static void checkFOURCC(uint8_t **data, const char *fourcc) {
	for (int i = 0; i < 4; ++i) {
		kinc_affirm(**data == fourcc[i]);
		++*data;
	}
}

static void readFOURCC(uint8_t **data, char *fourcc) {
	for (int i = 0; i < 4; ++i) {
		fourcc[i] = **data;
		++*data;
	}
	fourcc[4] = 0;
}

static void readChunk(uint8_t **data, struct WaveData *wave) {
	char fourcc[5];
	readFOURCC(data, fourcc);
	uint32_t chunksize = kinc_read_u32le(*data);
	*data += 4;
	if (strcmp(fourcc, "fmt ") == 0) {
		wave->audioFormat = kinc_read_u16le(*data + 0);
		wave->numChannels = kinc_read_u16le(*data + 2);
		wave->sampleRate = kinc_read_u32le(*data + 4);
		wave->bytesPerSecond = kinc_read_u32le(*data + 8);
		wave->bitsPerSample = kinc_read_u16le(*data + 14);
		*data += chunksize;
	}
	else if (strcmp(fourcc, "data") == 0) {
		wave->dataSize = chunksize;
		wave->data = (uint8_t *)malloc(chunksize * sizeof(uint8_t));
		kinc_affirm(wave->data != NULL);
		memcpy(wave->data, *data, chunksize);
		*data += chunksize;
	}
	else {
		*data += chunksize;
	}
}

static int16_t convert8to16(uint8_t sample) {
	return (sample - 127) << 8;
}

static void splitStereo8(uint8_t *data, int size, int16_t *left, int16_t *right) {
	for (int i = 0; i < size; ++i) {
		left[i] = convert8to16(data[i * 2 + 0]);
		right[i] = convert8to16(data[i * 2 + 1]);
	}
}

static void splitStereo16(int16_t *data, int size, int16_t *left, int16_t *right) {
	for (int i = 0; i < size; ++i) {
		left[i] = data[i * 2 + 0];
		right[i] = data[i * 2 + 1];
	}
}

static void splitMono8(uint8_t *data, int size, int16_t *left, int16_t *right) {
	for (int i = 0; i < size; ++i) {
		left[i] = convert8to16(data[i]);
		right[i] = convert8to16(data[i]);
	}
}

static void splitMono16(int16_t *data, int size, int16_t *left, int16_t *right) {
	for (int i = 0; i < size; ++i) {
		left[i] = data[i];
		right[i] = data[i];
	}
}

#define MAXIMUM_SOUNDS 1024
static kinc_a1_sound_t sounds[MAXIMUM_SOUNDS] = {0};

static kinc_a1_sound_t *find_sound(void) {
	for (int i = 0; i < MAXIMUM_SOUNDS; ++i) {
		if (!sounds[i].in_use) {
			return &sounds[i];
		}
	}
	return NULL;
}

kinc_a1_sound_t *kinc_a1_sound_create(const char *filename) {
	kinc_a1_sound_t *sound = find_sound();
	assert(sound != NULL);
	sound->in_use = true;
	sound->volume = 1.0f;
	sound->size = 0;
	sound->left = NULL;
	sound->right = NULL;
	size_t filenameLength = strlen(filename);
	uint8_t *data = NULL;

	if (strncmp(&filename[filenameLength - 4], ".ogg", 4) == 0) {
		kinc_file_reader_t file;
		if (!kinc_file_reader_open(&file, filename, KINC_FILE_TYPE_ASSET)) {
			sound->in_use = false;
			return NULL;
		}
		uint8_t *filedata = (uint8_t *)malloc(kinc_file_reader_size(&file));
		kinc_file_reader_read(&file, filedata, kinc_file_reader_size(&file));
		kinc_file_reader_close(&file);

		int channels, sample_rate;
		int samples = stb_vorbis_decode_memory(filedata, (int)kinc_file_reader_size(&file), &channels, &sample_rate, (short **)&data);
		sound->channel_count = (uint8_t)channels;
		sound->samples_per_second = (uint32_t)sample_rate;
		sound->size = samples * 2 * sound->channel_count;
		sound->bits_per_sample = 16;
		free(filedata);
	}
	else if (strncmp(&filename[filenameLength - 4], ".wav", 4) == 0) {
		struct WaveData wave = {0};
		{
			kinc_file_reader_t file;
			if (!kinc_file_reader_open(&file, filename, KINC_FILE_TYPE_ASSET)) {
				sound->in_use = false;
				return NULL;
			}
			uint8_t *filedata = (uint8_t *)malloc(kinc_file_reader_size(&file));
			kinc_file_reader_read(&file, filedata, kinc_file_reader_size(&file));
			kinc_file_reader_close(&file);
			uint8_t *data = filedata;

			checkFOURCC(&data, "RIFF");
			uint32_t filesize = kinc_read_u32le(data);
			data += 4;
			checkFOURCC(&data, "WAVE");
			while (data + 8 - filedata < (intptr_t)filesize) {
				readChunk(&data, &wave);
			}

			free(filedata);
		}

		sound->bits_per_sample = (uint8_t)wave.bitsPerSample;
		sound->channel_count = (uint8_t)wave.numChannels;
		sound->samples_per_second = wave.sampleRate;
		data = wave.data;
		sound->size = wave.dataSize;
	}
	else {
		assert(false);
	}

	if (sound->channel_count == 1) {
		if (sound->bits_per_sample == 8) {
			sound->left = (int16_t *)malloc(sound->size * sizeof(int16_t));
			sound->right = (int16_t *)malloc(sound->size * sizeof(int16_t));
			splitMono8(data, sound->size, sound->left, sound->right);
		}
		else if (sound->bits_per_sample == 16) {
			sound->size /= 2;
			sound->left = (int16_t *)malloc(sound->size * sizeof(int16_t));
			sound->right = (int16_t *)malloc(sound->size * sizeof(int16_t));
			splitMono16((int16_t *)data, sound->size, sound->left, sound->right);
		}
		else {
			kinc_affirm(false);
		}
	}
	else {
		// Left and right channel are in s16 audio stream, alternating.
		if (sound->bits_per_sample == 8) {
			sound->size /= 2;
			sound->left = (int16_t *)malloc(sound->size * sizeof(int16_t));
			sound->right = (int16_t *)malloc(sound->size * sizeof(int16_t));
			splitStereo8(data, sound->size, sound->left, sound->right);
		}
		else if (sound->bits_per_sample == 16) {
			sound->size /= 4;
			sound->left = (int16_t *)malloc(sound->size * sizeof(int16_t));
			sound->right = (int16_t *)malloc(sound->size * sizeof(int16_t));
			splitStereo16((int16_t *)data, sound->size, sound->left, sound->right);
		}
		else {
			kinc_affirm(false);
		}
	}
	sound->sample_rate_pos = 44100 / (float)sound->samples_per_second;
	free(data);

	return sound;
}

void kinc_a1_sound_destroy(kinc_a1_sound_t *sound) {
	free(sound->left);
	free(sound->right);
	sound->left = NULL;
	sound->right = NULL;
	sound->in_use = false;
}

float kinc_a1_sound_volume(kinc_a1_sound_t *sound) {
	return sound->volume;
}

void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value) {
	sound->volume = value;
}

static kinc_a1_sound_stream_t streams[256];
static int nextStream = 0;
static uint8_t buffer[1024 * 10];
static int bufferIndex;

kinc_a1_sound_stream_t *kinc_a1_sound_stream_create(const char *filename, bool looping) {
	kinc_a1_sound_stream_t *stream = &streams[nextStream];
	stream->myLooping = looping;
	stream->myVolume = 1;
	stream->rateDecodedHack = false;
	stream->end = false;
	kinc_file_reader_t file;
	kinc_file_reader_open(&file, filename, KINC_FILE_TYPE_ASSET);
	stream->buffer = &buffer[bufferIndex];
	bufferIndex += (int)kinc_file_reader_size(&file);
	uint8_t *filecontent = (uint8_t *)malloc(kinc_file_reader_size(&file));
	kinc_file_reader_read(&file, filecontent, kinc_file_reader_size(&file));
	kinc_file_reader_close(&file);
	memcpy(stream->buffer, filecontent, kinc_file_reader_size(&file));
	free(filecontent);
	stream->vorbis = stb_vorbis_open_memory(buffer, (int)kinc_file_reader_size(&file), NULL, NULL);
	if (stream->vorbis != NULL) {
		stb_vorbis_info info = stb_vorbis_get_info(stream->vorbis);
		stream->chans = info.channels;
		stream->rate = info.sample_rate;
	}
	else {
		stream->chans = 2;
		stream->rate = 22050;
	}
	++nextStream;
	return stream;
}

int kinc_a1_sound_stream_channels(kinc_a1_sound_stream_t *stream) {
	return stream->chans;
}

int kinc_a1_sound_stream_sample_rate(kinc_a1_sound_stream_t *stream) {
	return stream->rate;
}

bool kinc_a1_sound_stream_looping(kinc_a1_sound_stream_t *stream) {
	return stream->myLooping;
}

void kinc_a1_sound_stream_set_looping(kinc_a1_sound_stream_t *stream, bool loop) {
	stream->myLooping = loop;
}

float kinc_a1_sound_stream_volume(kinc_a1_sound_stream_t *stream) {
	return stream->myVolume;
}

void kinc_a1_sound_stream_set_volume(kinc_a1_sound_stream_t *stream, float value) {
	stream->myVolume = value;
}

bool kinc_a1_sound_stream_ended(kinc_a1_sound_stream_t *stream) {
	return stream->end;
}

float kinc_a1_sound_stream_length(kinc_a1_sound_stream_t *stream) {
	if (stream->vorbis == NULL)
		return 0;
	return stb_vorbis_stream_length_in_seconds(stream->vorbis);
}

float kinc_a1_sound_stream_position(kinc_a1_sound_stream_t *stream) {
	if (stream->vorbis == NULL)
		return 0;
	return stb_vorbis_get_sample_offset(stream->vorbis) / stb_vorbis_stream_length_in_samples(stream->vorbis) * kinc_a1_sound_stream_length(stream);
}

void kinc_a1_sound_stream_reset(kinc_a1_sound_stream_t *stream) {
	if (stream->vorbis != NULL)
		stb_vorbis_seek_start(stream->vorbis);
	stream->end = false;
	stream->rateDecodedHack = false;
}

float *kinc_a1_sound_stream_next_frame(kinc_a1_sound_stream_t *stream) {
	if (stream->vorbis == NULL) {
		for (int i = 0; i < stream->chans; ++i) {
			stream->samples[i] = 0;
		}
		return stream->samples;
	}
	if (stream->rate == 22050) {
		if (stream->rateDecodedHack) {
			stream->rateDecodedHack = false;
			return stream->samples;
		}
	}

	float left, right;
	float *samples_array[2] = {&left, &right};
	int read = stb_vorbis_get_samples_float(stream->vorbis, stream->chans, samples_array, 1);
	if (read == 0) {
		if (kinc_a1_sound_stream_looping(stream)) {
			stb_vorbis_seek_start(stream->vorbis);
			stb_vorbis_get_samples_float(stream->vorbis, stream->chans, samples_array, 1);
		}
		else {
			stream->end = true;
			for (int i = 0; i < stream->chans; ++i) {
				stream->samples[i] = 0;
			}
			return stream->samples;
		}
	}

	stream->samples[0] = samples_array[0][0];
	stream->samples[1] = samples_array[1][0];

	stream->rateDecodedHack = true;
	return stream->samples;
}

#endif
