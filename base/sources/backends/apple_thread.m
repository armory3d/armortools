#include <stdio.h>
#include <string.h>
#include <Foundation/Foundation.h>
#include <iron_thread.h>
#include <pthread.h>
#include <stdio.h>
#include <wchar.h>
#include "posix_thread.c"

static void *ThreadProc(void *arg) {
	@autoreleasepool {
		iron_thread_t *t = (iron_thread_t *)arg;
		t->impl.thread(t->impl.param);
		pthread_exit(NULL);
		return NULL;
	}
}

void iron_thread_init(iron_thread_t *t, void (*thread)(void *param), void *param) {
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

void iron_thread_wait_and_destroy(iron_thread_t *thread) {
	int ret;
	do {
		ret = pthread_join(thread->impl.pthread, NULL);
	} while (ret != 0);
}

bool iron_thread_try_to_destroy(iron_thread_t *thread) {
	return pthread_join(thread->impl.pthread, NULL) == 0;
}

void iron_threads_init(void) {}

void iron_threads_quit(void) {}

int iron_hardware_threads(void) {
	return (int)[[NSProcessInfo processInfo] processorCount];
}

int iron_cpu_cores(void) {
	return iron_hardware_threads();
}
