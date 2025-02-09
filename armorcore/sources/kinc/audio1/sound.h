#pragma once

#include <kinc/global.h>
#include <kinc/audio2/audio.h>
#include <stdint.h>

/*! \file sound.h
    \brief Sounds are pre-decoded on load and therefore primarily useful for playing sound-effects.
*/

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
