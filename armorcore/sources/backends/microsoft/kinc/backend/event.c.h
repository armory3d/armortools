#include <kinc/threads/event.h>

void kinc_event_init(kinc_event_t *event, bool auto_clear) {
	event->impl.event = CreateEvent(0, auto_clear ? FALSE : TRUE, 0, 0);
}

void kinc_event_destroy(kinc_event_t *event) {
	CloseHandle(event->impl.event);
}

void kinc_event_signal(kinc_event_t *event) {
	SetEvent(event->impl.event);
}

void kinc_event_wait(kinc_event_t *event) {
	WaitForSingleObject(event->impl.event, INFINITE);
}

bool kinc_event_try_to_wait(kinc_event_t *event, double seconds) {
	return WaitForSingleObject(event->impl.event, (DWORD)(seconds * 1000.0)) != WAIT_TIMEOUT;
}

void kinc_event_reset(kinc_event_t *event) {
	ResetEvent(event->impl.event);
}
