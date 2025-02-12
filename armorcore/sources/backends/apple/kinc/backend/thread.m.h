#include <stdio.h>
#include <string.h>
#include <Foundation/Foundation.h>
#include <kinc/threads/mutex.h>
#include <kinc/threads/thread.h>
#include <pthread.h>
#include <stdio.h>
#include <wchar.h>

static void *ThreadProc(void *arg) {
	@autoreleasepool {
		kinc_thread_t *t = (kinc_thread_t *)arg;
		t->impl.thread(t->impl.param);
		pthread_exit(NULL);
		return NULL;
	}
}

void kinc_thread_init(kinc_thread_t *t, void (*thread)(void *param), void *param) {
	t->impl.param = param;
	t->impl.thread = thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// pthread_attr_setstacksize(&attr, 1024 * 64);
	struct sched_param sp;
	memset(&sp, 0, sizeof(sp));
	sp.sched_priority = 0;
	pthread_attr_setschedparam(&attr, &sp);
	pthread_create(&t->impl.pthread, &attr, &ThreadProc, t);
	pthread_attr_destroy(&attr);
}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {
	int ret;
	do {
		ret = pthread_join(thread->impl.pthread, NULL);
	} while (ret != 0);
}

bool kinc_thread_try_to_destroy(kinc_thread_t *thread) {
	return pthread_join(thread->impl.pthread, NULL) == 0;
}

void kinc_threads_init(void) {}

void kinc_threads_quit(void) {}
