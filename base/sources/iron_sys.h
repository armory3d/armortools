#pragma once

#include "iron_array.h"
#include "iron_global.h"
#include "iron_gpu.h"
#include "iron_map.h"
#include "iron_system.h"
#include <stdbool.h>

extern any_map_t *_sys_shaders;
extern f32        _sys_start_time;

extern any_array_t *_sys_on_next_frames;
extern any_array_t *_sys_on_end_frames;
extern any_array_t *_sys_on_updates;
extern any_array_t *_sys_on_renders;
extern i32          _sys_lastw;
extern i32          _sys_lasth;

extern void (*sys_on_resize)(void);
extern i32  (*sys_on_w)(void);
extern i32  (*sys_on_h)(void);
extern i32  (*sys_on_x)(void);
extern i32  (*sys_on_y)(void);

typedef struct sys_callback {
	void (*f)(void);
} sys_callback_t;

typedef struct sys_string_callback {
	void (*f)(char *s);
} sys_string_callback_t;

typedef struct callback {
	void (*f)(void *data);
	void *data;
} callback_t;

extern any_array_t *_sys_foreground_listeners;
extern any_array_t *_sys_resume_listeners;
extern any_array_t *_sys_pause_listeners;
extern any_array_t *_sys_background_listeners;
extern any_array_t *_sys_shutdown_listeners;
extern any_array_t *_sys_drop_files_listeners;

void  sys_start(iron_window_options_t *ops);
f32   sys_time(void);
f32   sys_delta(void);
f32   sys_real_delta(void);
i32   sys_w(void);
i32   sys_h(void);
i32   sys_x(void);
i32   sys_y(void);
char *sys_title(void);
void  sys_title_set(char *value);
i32   sys_display_primary_id(void);
i32   sys_display_width(void);
i32   sys_display_height(void);
i32   sys_display_frequency(void);
i32   sys_display_ppi(void);
char *sys_shader_ext(void);

gpu_shader_t *sys_get_shader(char *name);

void sys_notify_on_app_state(void (*on_foreground)(void), void (*on_resume)(void), void (*on_pause)(void), void (*on_background)(void),
                             void (*on_shutdown)(void));
void sys_notify_on_drop_files(void (*drop_files_listener)(char *s));
void sys_notify_on_update(void (*f)(void *data), void *data);
void sys_notify_on_render(void (*f)(void *data), void *data);
void sys_notify_on_next_frame(void (*f)(void *data), void *data);
void sys_notify_on_end_frame(void (*f)(void *data), void *data);
void sys_remove_update(void (*f)(void *data));
void sys_remove_render(void (*f)(void *data));
void sys_remove_end_frame(void (*f)(void *data));

void sys_render(void);
void sys_foreground(void);
void sys_resume(void);
void sys_pause(void);
void sys_background(void);
void sys_shutdown(void);
void sys_drop_files(char *file_path);

void sys_foreground_callback(void);
void sys_resume_callback(void);
void sys_pause_callback(void);
void sys_background_callback(void);
void sys_shutdown_callback(void);
void sys_drop_files_callback(char *file_path);
void sys_keyboard_down_callback(i32 code);
void sys_keyboard_up_callback(i32 code);
void sys_mouse_down_callback(i32 button, i32 x, i32 y);
void sys_mouse_up_callback(i32 button, i32 x, i32 y);
void sys_mouse_move_callback(i32 x, i32 y, i32 mx, i32 my);
void sys_mouse_wheel_callback(f32 delta);
void sys_touch_down_callback(i32 index, i32 x, i32 y);
void sys_touch_up_callback(i32 index, i32 x, i32 y);
void sys_touch_move_callback(i32 index, i32 x, i32 y);
void sys_pen_down_callback(i32 x, i32 y, f32 pressure);
void sys_pen_up_callback(i32 x, i32 y, f32 pressure);
void sys_pen_move_callback(i32 x, i32 y, f32 pressure);

#ifdef WITH_GAMEPAD
void sys_gamepad_axis_callback(i32 gamepad, i32 axis, f32 value);
void sys_gamepad_button_callback(i32 gamepad, i32 button, f32 value);
#endif

char *data_path(void);
void  scene_render_frame(void);

typedef struct {
	void *video_;
} video_t;

void video_unload(video_t *self);

char *sys_buffer_to_string(buffer_t *b);
buffer_t *sys_string_to_buffer(char *str);
