#include <kinc/threads/fiber.h>

VOID WINAPI fiber_func(LPVOID param) {
	kinc_fiber_t *fiber = (kinc_fiber_t *)param;
	fiber->impl.func(fiber->impl.param);
}

void kinc_fiber_init_current_thread(kinc_fiber_t *fiber) {
	fiber->impl.fiber = ConvertThreadToFiber(NULL);
}

void kinc_fiber_init(kinc_fiber_t *fiber, void (*func)(void *param), void *param) {
	fiber->impl.func = func;
	fiber->impl.param = param;
	fiber->impl.fiber = CreateFiber(0, fiber_func, fiber);
}

void kinc_fiber_destroy(kinc_fiber_t *fiber) {
	DeleteFiber(fiber->impl.fiber);
	fiber->impl.fiber = NULL;
}

void kinc_fiber_switch(kinc_fiber_t *fiber) {
	SwitchToFiber(fiber->impl.fiber);
}
