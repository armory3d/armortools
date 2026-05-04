#include <iron_thread.h>

// js creates a worker sharing the same wasm memory, then calls wasm_thread_run
__attribute__((import_module("imports"), import_name("js_thread_create"))) void js_thread_create(void (*func)(void *), void *param, volatile int32_t *done);

void iron_thread_init(iron_thread_t *t, void (*thread)(void *param), void *param) {
	__atomic_store_n(&t->impl.done, 0, __ATOMIC_SEQ_CST);
	js_thread_create(thread, param, &t->impl.done);
}

void iron_thread_wait_and_destroy(iron_thread_t *thread) {
	while (__atomic_load_n(&thread->impl.done, __ATOMIC_SEQ_CST) == 0) {
	}
}

bool iron_thread_try_to_destroy(iron_thread_t *thread) {
	return __atomic_load_n(&thread->impl.done, __ATOMIC_SEQ_CST) != 0;
}

void iron_threads_init() {}

void iron_threads_quit() {}

void iron_thread_set_name(const char *name) {}

void iron_mutex_init(iron_mutex_t *mutex) {
	__atomic_store_n(&mutex->impl.state, 0, __ATOMIC_SEQ_CST);
}

void iron_mutex_destroy(iron_mutex_t *mutex) {}

bool iron_mutex_try_to_lock(iron_mutex_t *mutex) {
	int32_t expected = 0;
	return __atomic_compare_exchange_n(&mutex->impl.state, &expected, 1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

void iron_mutex_lock(iron_mutex_t *mutex) {
	while (!iron_mutex_try_to_lock(mutex)) {
	}
}

void iron_mutex_unlock(iron_mutex_t *mutex) {
	__atomic_store_n(&mutex->impl.state, 0, __ATOMIC_SEQ_CST);
}

__attribute__((export_name("wasm_thread_run"))) void wasm_thread_run(void (*func)(void *), void *param, volatile int32_t *done) {
	func(param);
	__atomic_store_n(done, 1, __ATOMIC_SEQ_CST);
}
