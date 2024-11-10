#include <kinc/video.h>

void kinc_video_init(kinc_video_t *video, const char *filename) {}

void kinc_video_destroy(kinc_video_t *video) {}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	return NULL;
}

int kinc_video_width(kinc_video_t *video) {
	return 64;
}

int kinc_video_height(kinc_video_t *video) {
	return 64;
}

void kinc_video_play(kinc_video_t *video, bool loop) {}

void kinc_video_pause(kinc_video_t *video) {}

void kinc_video_stop(kinc_video_t *video) {}

void kinc_video_update(kinc_video_t *video, double time) {}

double kinc_video_duration(kinc_video_t *video) {
	return 0.0;
}

double kinc_video_position(kinc_video_t *video) {
	return 0.0;
}

bool kinc_video_finished(kinc_video_t *video) {
	return true;
}

bool kinc_video_paused(kinc_video_t *video) {
	return true;
}

void kinc_internal_video_sound_stream_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency) {}

void kinc_internal_video_sound_stream_destroy(kinc_internal_video_sound_stream_t *stream) {}

void kinc_internal_video_sound_stream_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count) {}

static float samples[2];

float *kinc_internal_video_sound_stream_next_frame(kinc_internal_video_sound_stream_t *stream) {
	samples[0] = 0.0f;
	samples[1] = 0.0f;
	return samples;
}

bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream) {
	return true;
}
