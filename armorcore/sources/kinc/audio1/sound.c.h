#include "sound.h"

#define STB_VORBIS_HEADER_ONLY
#include <kinc/libs/stb_vorbis.c>

#include <kinc/audio2/audio.h>
#include <kinc/error.h>
#include <kinc/io/filereader.h>

#include <assert.h>
#include <string.h>

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
