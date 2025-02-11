#pragma once

#include <kinc/global.h>
#include <kinc/backend/video.h>
#include <kinc/graphics4/texture.h>

/*! \file video.h
    \brief Hardware-assisted video decoding support. Actually supported
    formats vary per target-system.
*/

typedef struct kinc_video {
	kinc_video_impl_t impl;
} kinc_video_t;

const char **kinc_video_formats(void);

void kinc_video_init(kinc_video_t *video, const char *filename);
void kinc_video_destroy(kinc_video_t *video);
void kinc_video_play(kinc_video_t *video, bool loop);
void kinc_video_pause(kinc_video_t *video);
void kinc_video_stop(kinc_video_t *video);
int kinc_video_width(kinc_video_t *video);
int kinc_video_height(kinc_video_t *video);
kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video);
double kinc_video_duration(kinc_video_t *video);
double kinc_video_position(kinc_video_t *video);
bool kinc_video_finished(kinc_video_t *video);
bool kinc_video_paused(kinc_video_t *video);
void kinc_video_update(kinc_video_t *video, double time);
