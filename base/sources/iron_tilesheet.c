#include "iron_tilesheet.h"

#include "iron_gc.h"
#include <math.h>
#include <string.h>

tilesheet_data_t *tilesheet_datas       = NULL;
i32               tilesheet_datas_count = 0;

f32 sys_real_delta(void);
f32 math_floor(f32 x);

tilesheet_t *tilesheet_create(char *scene_name, char *tilesheet_ref, char *tilesheet_action_ref) {
	tilesheet_t *t = gc_alloc(sizeof(tilesheet_t));
	t->ready       = false;
	for (i32 i = 0; i < tilesheet_datas_count; ++i) {
		tilesheet_data_t *ts = &tilesheet_datas[i];
		if (strcmp(ts->name, tilesheet_ref) == 0) {
			t->raw = ts;
			tilesheet_play(t, tilesheet_action_ref, NULL);
			t->ready = true;
			break;
		}
	}
	return t;
}

void tilesheet_play(tilesheet_t *self, char *action_ref, void (*on_action_complete)(void)) {
	self->on_action_complete = on_action_complete;
	for (i32 i = 0; i < self->raw->actions_count; ++i) {
		tilesheet_action_t *a = &self->raw->actions[i];
		if (strcmp(a->name, action_ref) == 0) {
			self->action = a;
			break;
		}
	}
	tilesheet_set_frame(self, self->action->start);
	self->paused = false;
	self->time   = 0.0f;
}

void tilesheet_pause(tilesheet_t *self) {
	self->paused = true;
}

void tilesheet_resume(tilesheet_t *self) {
	self->paused = false;
}

void tilesheet_set_frame_offset(tilesheet_t *self, i32 frame) {
	tilesheet_set_frame(self, self->action->start + frame);
	self->paused = false;
}

i32 tilesheet_get_frame_offset(tilesheet_t *self) {
	return self->frame - self->action->start;
}

void tilesheet_update(tilesheet_t *self) {
	if (!self->ready || self->paused) {
		return;
	}

	self->time += sys_real_delta();

	f32 frame_time        = 1.0f / self->raw->framerate;
	i32 frames_to_advance = 0;

	while (self->time >= frame_time) {
		self->time -= frame_time;
		frames_to_advance++;
	}

	if (frames_to_advance != 0) {
		tilesheet_set_frame(self, self->frame + frames_to_advance);
	}
}

void tilesheet_set_frame(tilesheet_t *self, i32 f) {
	self->frame = f;

	// Action end
	if (self->frame > self->action->end) {
		if (self->on_action_complete != NULL) {
			self->on_action_complete();
		}
		if (self->action->loop) {
			tilesheet_set_frame(self, self->action->start);
		}
		else {
			self->paused = true;
		}
		return;
	}

	i32 tx       = self->frame % self->raw->tiles_x;
	i32 ty       = (i32)math_floor((f32)self->frame / self->raw->tiles_x);
	self->tile_x = tx * (1.0f / self->raw->tiles_x);
	self->tile_y = ty * (1.0f / self->raw->tiles_y);
}
