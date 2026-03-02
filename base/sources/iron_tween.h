#pragma once

#include "iron_array.h"
#include "iron_global.h"
#include <stdbool.h>

typedef enum {
	EASE_LINEAR,
	EASE_EXPO_IN,
	EASE_EXPO_OUT
} ease_t;

typedef struct tween_anim {
	f32   *target;
	f32    to;
	f32    duration;
	f32    delay;
	ease_t ease;
	void (*done)(void *data);
	void (*tick)(void);
	void *done_data;
	f32   _from;
	f32   _time;
} tween_anim_t;

tween_anim_t *tween_to(tween_anim_t *anim);
tween_anim_t *tween_timer(f32 delay, void (*done)(void *data), void *data);
void          tween_stop(tween_anim_t *anim);
void          tween_reset(void);
void          tween_update(void *unused);
