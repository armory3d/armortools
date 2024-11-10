#include "audio.h"

#include <stdbool.h>

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

#include "audio.c.h"
#include "sound.c.h"
#include "soundstream.c.h"
