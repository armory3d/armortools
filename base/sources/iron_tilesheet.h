#pragma once

#include "iron_global.h"
#include <stdbool.h>

typedef struct tilesheet_action {
	char *name;
	i32   start;
	i32   end;
	bool  loop;
} tilesheet_action_t;

typedef struct tilesheet_data {
	char               *name;
	i32                 tiles_x;
	i32                 tiles_y;
	i32                 framerate;
	tilesheet_action_t *actions;
	i32                 actions_count;
} tilesheet_data_t;

typedef struct tilesheet {
	f32                 tile_x; // Tile offset on tilesheet texture 0-1
	f32                 tile_y;
	tilesheet_data_t   *raw;
	tilesheet_action_t *action;
	bool                ready;
	bool                paused;
	i32                 frame;
	f32                 time;
	void (*on_action_complete)(void);
} tilesheet_t;

extern tilesheet_data_t *tilesheet_datas;
extern i32               tilesheet_datas_count;

tilesheet_t *tilesheet_create(char *scene_name, char *tilesheet_ref, char *tilesheet_action_ref);
void         tilesheet_play(tilesheet_t *self, char *action_ref, void (*on_action_complete)(void));
void         tilesheet_pause(tilesheet_t *self);
void         tilesheet_resume(tilesheet_t *self);
void         tilesheet_set_frame_offset(tilesheet_t *self, i32 frame);
i32          tilesheet_get_frame_offset(tilesheet_t *self);
void         tilesheet_update(tilesheet_t *self);
void         tilesheet_set_frame(tilesheet_t *self, i32 f);
