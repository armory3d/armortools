#pragma once

#include <kinc/global.h>

#include <kinc/audio2/audio.h>

#include <stdint.h>

/*! \file sound.h
    \brief Sounds are pre-decoded on load and therefore primarily useful for playing sound-effects.
*/

#ifdef __cplusplus
extern "C" {
#endif

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

/// <summary>
/// Create a sound from a wav file.
/// </summary>
/// <param name="filename">Path to a wav file</param>
/// <returns>The newly created sound</returns>
kinc_a1_sound_t *kinc_a1_sound_create(const char *filename);

/// <summary>
/// Destroy a sound.
/// </summary>
/// <param name="sound">The sound to destroy.</param>
void kinc_a1_sound_destroy(kinc_a1_sound_t *sound);

/// <summary>
/// Gets the current volume-multiplicator that's used when playing the sound.
/// </summary>
/// <param name="sound">The sound to query</param>
/// <returns>The volume-multiplicator</returns>
float kinc_a1_sound_volume(kinc_a1_sound_t *sound);

/// <summary>
/// Sets the volume-multiplicator that's used when playing the sound.
/// </summary>
/// <param name="sound">The sound to modify</param>
/// <param name="value">The volume-multiplicator to set</param>
void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value);

#ifdef __cplusplus
}
#endif
