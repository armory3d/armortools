#include <kinc/threads/event.h>

void kinc_event_init(kinc_event_t *event, bool auto_reset) {}

void kinc_event_destroy(kinc_event_t *event) {}

void kinc_event_signal(kinc_event_t *event) {}

void kinc_event_wait(kinc_event_t *event) {}

bool kinc_event_try_to_wait(kinc_event_t *event, double seconds) {
	return false;
}

void kinc_event_reset(kinc_event_t *event) {}
