#pragma once

#include <kinc/global.h>

#include <kinc/backend/video.h>
#include <kinc/graphics4/texture.h>

/*! \file video.h
    \brief Hardware-assisted video decoding support. Actually supported
    formats vary per target-system.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_video {
	kinc_video_impl_t impl;
} kinc_video_t;

/// <summary>
/// Returns the list of video formats which are supported on the current system.
/// </summary>
/// <returns>A NULL-terminated list of strings representing the supported video-formats</returns>
const char **kinc_video_formats(void);

/// <summary>
/// Prepares a video object based on an encoded video-file.
/// </summary>
void kinc_video_init(kinc_video_t *video, const char *filename);

/// <summary>
/// Destroys a video object.
/// </summary>
void kinc_video_destroy(kinc_video_t *video);

/// <summary>
/// Starts playing a video.
/// </summary>
void kinc_video_play(kinc_video_t *video, bool loop);

/// <summary>
/// Pauses a video.
/// </summary>
void kinc_video_pause(kinc_video_t *video);

/// <summary>
/// Stops a video which is equivalent to pausing it and setting the position to 0.
/// </summary>
void kinc_video_stop(kinc_video_t *video);

/// <summary>
/// Gets the width of the video in pixels.
/// </summary>
/// <returns>The width of the video in pixels</returns>
int kinc_video_width(kinc_video_t *video);

/// <summary>
/// Gets the height of the video in pixels.
/// </summary>
/// <returns>The height of the video in pixels</returns>
int kinc_video_height(kinc_video_t *video);

/// <summary>
/// Gets the current image of a playing video which can be rendered using any of Kinc's graphics APIs.
/// </summary>
/// <returns>The current image of a playing video</returns>
kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video);

/// <summary>
/// Gets the duration of the video in seconds.
/// </summary>
/// <returns>The duration of the video in seconds</returns>
double kinc_video_duration(kinc_video_t *video);

/// <summary>
/// Returns the current playback-position of the video in seconds.
/// </summary>
/// <returns>The current playback-position in seconds</returns>
double kinc_video_position(kinc_video_t *video);

/// <summary>
/// Returns whether the video reached its end.
/// </summary>
/// <returns>the end-state of the video</returns>
bool kinc_video_finished(kinc_video_t *video);

/// <summary>
/// Returns whether the video is currently paused.
/// </summary>
/// <returns>The current pause state of the video</returns>
bool kinc_video_paused(kinc_video_t *video);

/// <summary>
/// Call this every frame to update the video. This is not required on all targets but where it's not required the function just does nothing - so please call
/// it.
/// </summary>
void kinc_video_update(kinc_video_t *video, double time);

#ifdef __cplusplus
}
#endif
