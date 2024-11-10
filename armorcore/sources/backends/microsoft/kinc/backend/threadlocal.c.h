#include <kinc/threads/threadlocal.h>

void kinc_thread_local_init(kinc_thread_local_t *local) {
	local->impl.slot = TlsAlloc();
	TlsSetValue(local->impl.slot, 0);
}

void kinc_thread_local_destroy(kinc_thread_local_t *local) {
	TlsFree(local->impl.slot);
}

void *kinc_thread_local_get(kinc_thread_local_t *local) {
	return TlsGetValue(local->impl.slot);
}

void kinc_thread_local_set(kinc_thread_local_t *local, void *data) {
	TlsSetValue(local->impl.slot, data);
}
