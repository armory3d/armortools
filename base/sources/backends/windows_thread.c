#include <iron_thread.h>
#include <Windows.h>

void iron_threads_init() {}

void iron_threads_quit() {}

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

void iron_thread_init(iron_thread_t *thread, void (*func)(void *param), void *param) {
	thread->impl.func = func;
	thread->impl.param = param;

	intptr_t start_index = thread_start_index++;
	if (thread_start_index >= THREAD_STARTS) {
		thread_start_index = 0;
	}
	starts[start_index].thread = func;
	starts[start_index].param = param;
	thread->impl.handle = CreateThread(0, 65536, ThreadProc, (LPVOID)start_index, 0, 0);
}

void iron_thread_wait_and_destroy(iron_thread_t *thread) {
	WaitForSingleObject(thread->impl.handle, INFINITE);
	CloseHandle(thread->impl.handle);
}

bool iron_thread_try_to_destroy(iron_thread_t *thread) {
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

void iron_thread_set_name(const char *name) {
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

void iron_thread_sleep(int milliseconds) {
	Sleep(milliseconds);
}

void iron_mutex_init(iron_mutex_t *mutex) {
	InitializeCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

void iron_mutex_destroy(iron_mutex_t *mutex) {
	DeleteCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

void iron_mutex_lock(iron_mutex_t *mutex) {
	EnterCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

bool iron_mutex_try_to_lock(iron_mutex_t *mutex) {
	return TryEnterCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}

void iron_mutex_unlock(iron_mutex_t *mutex) {
	LeaveCriticalSection((LPCRITICAL_SECTION)&mutex->impl.criticalSection);
}
