#include <iron_video.h>

void iron_video_init(iron_video_t *video, const char *filename) {}

void iron_video_destroy(iron_video_t *video) {}

void iron_video_play(iron_video_t *video, bool loop) {}

void iron_video_pause(iron_video_t *video) {}

void iron_video_stop(iron_video_t *video) {}

int iron_video_width(iron_video_t *video) {
	return 256;
}

int iron_video_height(iron_video_t *video) {
	return 256;
}

gpu_texture_t *iron_video_current_image(iron_video_t *video) {
	return NULL;
}

double iron_video_duration(iron_video_t *video) {
	return 0.0;
}

double iron_video_position(iron_video_t *video) {
	return 0.0;
}

bool iron_video_finished(iron_video_t *video) {
	return false;
}

bool iron_video_paused(iron_video_t *video) {
	return false;
}

void iron_video_update(iron_video_t *video, double time) {}

void iron_internal_video_sound_stream_init(iron_internal_video_sound_stream_t *stream, int channel_count, int frequency) {}

void iron_internal_video_sound_stream_destroy(iron_internal_video_sound_stream_t *stream) {}

void iron_internal_video_sound_stream_insert_data(iron_internal_video_sound_stream_t *stream, float *data, int sample_count) {}

static float samples[2] = {0};

float *iron_internal_video_sound_stream_next_frame(iron_internal_video_sound_stream_t *stream) {
	return samples;
}

bool iron_internal_video_sound_stream_ended(iron_internal_video_sound_stream_t *stream) {
	return true;
}
