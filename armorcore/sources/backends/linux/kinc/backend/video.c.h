#include <kinc/video.h>

#if !defined(KINC_VIDEO_GSTREAMER)
void kinc_video_init(kinc_video_t *video, const char *filename) {}

void kinc_video_destroy(kinc_video_t *video) {}

void kinc_video_play(kinc_video_t *video, bool loop) {}

void kinc_video_pause(kinc_video_t *video) {}

void kinc_video_stop(kinc_video_t *video) {}

int kinc_video_width(kinc_video_t *video) {
	return 256;
}

int kinc_video_height(kinc_video_t *video) {
	return 256;
}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	return NULL;
}

double kinc_video_duration(kinc_video_t *video) {
	return 0.0;
}

double kinc_video_position(kinc_video_t *video) {
	return 0.0;
}

bool kinc_video_finished(kinc_video_t *video) {
	return false;
}

bool kinc_video_paused(kinc_video_t *video) {
	return false;
}

void kinc_video_update(kinc_video_t *video, double time) {}

void kinc_internal_video_sound_stream_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency) {}

void kinc_internal_video_sound_stream_destroy(kinc_internal_video_sound_stream_t *stream) {}

void kinc_internal_video_sound_stream_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count) {}

static float samples[2] = {0};

float *kinc_internal_video_sound_stream_next_frame(kinc_internal_video_sound_stream_t *stream) {
	return samples;
}

bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream) {
	return true;
}
#endif
