#pragma once

#include <kinc/global.h>

#include <stdbool.h>
#include <stdint.h>

/*! \file soundstream.h
    \brief Sound-Streams are decoded while playing and as such are useful for large audio-files like music or speech.
*/

#ifdef __cplusplus
extern "C" {
#endif

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

/// <summary>
/// Create a sound-stream from a wav file.
/// </summary>
/// <param name="filename">A path to a wav file</param>
/// <param name="looping">Defines whether the stream will be looped automatically</param>
/// <returns>The newly created sound-stream</returns>
kinc_a1_sound_stream_t *kinc_a1_sound_stream_create(const char *filename, bool looping);

/// <summary>
/// Gets the next audio-frame in the stream.
/// </summary>
/// <param name="stream">The stream to extract the frame from</param>
/// <returns>The next sample</returns>
float *kinc_a1_sound_stream_next_frame(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Gets the number of audio-channels the stream uses.
/// </summary>
/// <param name="stream">The stream to extract the number of channels from</param>
/// <returns>The number of audio-channels</returns>
int kinc_a1_sound_stream_channels(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Gets the sample-rate used by the stream.
/// </summary>
/// <param name="stream">The stream to extract the sample-rate from</param>
/// <returns>The sample-rate of the stream</returns>
int kinc_a1_sound_stream_sample_rate(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Returns whether the stream loops automatically.
/// </summary>
/// <param name="stream">The stream to extract the looping-information from</param>
/// <returns>Whether the stream loops</returns>
bool kinc_a1_sound_stream_looping(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Changes whether the stream is looped automatically.
/// </summary>
/// <param name="stream">The stream to change</param>
/// <param name="loop">The new loop value to set</param>
void kinc_a1_sound_stream_set_looping(kinc_a1_sound_stream_t *stream, bool loop);

/// <summary>
/// Returns whether the stream finished playing.
/// </summary>
/// <param name="stream">The stream to query</param>
/// <returns>Whether the stream finished playing</returns>
bool kinc_a1_sound_stream_ended(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Returns the length of the stream.
/// </summary>
/// <param name="stream">The stream to query</param>
/// <returns>The length of the stream in seconds</returns>
float kinc_a1_sound_stream_length(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Returns the current playing-position of the stream.
/// </summary>
/// <param name="stream">The stream to query</param>
/// <returns>The current playing-position in seconds</returns>
float kinc_a1_sound_stream_position(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Resets the stream to its start-position.
/// </summary>
/// <param name="stream">The stream to change</param>
void kinc_a1_sound_stream_reset(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Gets the stream's volume-multiplicator.
/// </summary>
/// <param name="stream">The stream to query</param>
/// <returns>The volume-multiplicator</returns>
float kinc_a1_sound_stream_volume(kinc_a1_sound_stream_t *stream);

/// <summary>
/// Sets the stream's volume-multiplicator.
/// </summary>
/// <param name="stream">The stream to change</param>
/// <param name="value">The volume-multiplicator</param>
void kinc_a1_sound_stream_set_volume(kinc_a1_sound_stream_t *stream, float value);

#ifdef __cplusplus
}
#endif
