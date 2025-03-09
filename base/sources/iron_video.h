// #pragma once

// #include <iron_global.h>
// #include <iron_gpu.h>
// #include BACKEND_VIDEO_H

// typedef struct iron_video {
// 	iron_video_impl_t impl;
// } iron_video_t;

// const char **iron_video_formats(void);

// void iron_video_init(iron_video_t *video, const char *filename);
// void iron_video_destroy(iron_video_t *video);
// void iron_video_play(iron_video_t *video, bool loop);
// void iron_video_pause(iron_video_t *video);
// void iron_video_stop(iron_video_t *video);
// int iron_video_width(iron_video_t *video);
// int iron_video_height(iron_video_t *video);
// iron_g5_texture_t *iron_video_current_image(iron_video_t *video);
// double iron_video_duration(iron_video_t *video);
// double iron_video_position(iron_video_t *video);
// bool iron_video_finished(iron_video_t *video);
// bool iron_video_paused(iron_video_t *video);
// void iron_video_update(iron_video_t *video, double time);
