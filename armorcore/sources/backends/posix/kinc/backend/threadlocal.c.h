#include <kinc/threads/threadlocal.h>

void kinc_thread_local_init(kinc_thread_local_t *local) {
	pthread_key_create(&local->impl.key, NULL);
}

void kinc_thread_local_destroy(kinc_thread_local_t *local) {
	pthread_key_delete(local->impl.key);
}

void *kinc_thread_local_get(kinc_thread_local_t *local) {
	return pthread_getspecific(local->impl.key);
}

void kinc_thread_local_set(kinc_thread_local_t *local, void *data) {
	pthread_setspecific(local->impl.key, data);
}
