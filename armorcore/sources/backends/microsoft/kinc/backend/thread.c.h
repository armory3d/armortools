#include <kinc/threads/thread.h>

void kinc_threads_init() {}

void kinc_threads_quit() {}

struct thread_start {
	void (*thread)(void *param);
	void *param;
};

#define THREAD_STARTS 64
static struct thread_start starts[THREAD_STARTS];
static int thread_start_index = 0;

static DWORD WINAPI ThreadProc(LPVOID arg) {
	intptr_t start_index = (intptr_t)arg;
	starts[start_index].thread(starts[start_index].param);
	return 0;
}

void kinc_thread_init(kinc_thread_t *thread, void (*func)(void *param), void *param) {
	thread->impl.func = func;
	thread->impl.param = param;

	intptr_t start_index = thread_start_index++;
	if (thread_start_index >= THREAD_STARTS) {
		thread_start_index = 0;
	}
	starts[start_index].thread = func;
	starts[start_index].param = param;
	thread->impl.handle = CreateThread(0, 65536, ThreadProc, (LPVOID)start_index, 0, 0);
	assert(thread->impl.handle != NULL);
}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {
	WaitForSingleObject(thread->impl.handle, INFINITE);
	CloseHandle(thread->impl.handle);
}

bool kinc_thread_try_to_destroy(kinc_thread_t *thread) {
	DWORD code;
	GetExitCodeThread(thread->impl.handle, &code);
	if (code != STILL_ACTIVE) {
		CloseHandle(thread->impl.handle);
		return true;
	}
	return false;
}

typedef HRESULT(WINAPI *SetThreadDescriptionType)(HANDLE hThread, PCWSTR lpThreadDescription);
static SetThreadDescriptionType MySetThreadDescription = NULL;
static bool set_thread_description_loaded = false;

void kinc_thread_set_name(const char *name) {
	if (!set_thread_description_loaded) {
		HMODULE kernel32 = LoadLibraryA("kernel32.dll");
		MySetThreadDescription = (SetThreadDescriptionType)GetProcAddress(kernel32, "SetThreadDescription");
		set_thread_description_loaded = true;
	}

	if (MySetThreadDescription != NULL) {
		wchar_t wide_name[256];
		MultiByteToWideChar(CP_ACP, 0, name, -1, wide_name, 256);
		MySetThreadDescription(GetCurrentThread(), wide_name);
	}
}

void kinc_thread_sleep(int milliseconds) {
	Sleep(milliseconds);
}
