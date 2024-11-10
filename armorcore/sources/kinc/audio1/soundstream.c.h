#include "soundstream.h"

#define STB_VORBIS_HEADER_ONLY
#include <kinc/libs/stb_vorbis.c>

#include <kinc/io/filereader.h>

#include <stdlib.h>
#include <string.h>

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
