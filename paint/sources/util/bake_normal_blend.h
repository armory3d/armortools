#pragma once

// Launch Blender directly: paths and object names never pass through a shell.
#ifdef IRON_WINDOWS
#include <windows.h>
static wchar_t *bn_wide(const char *text) {
	int      count = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t *wide  = calloc(count, sizeof(wchar_t));
	if (wide)
		MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, count);
	return wide;
}
static FILE *bn_open(const char *path, const char *mode) {
	wchar_t *p = bn_wide(path), *m = bn_wide(mode);
	FILE    *file = p && m ? _wfopen(p, m) : NULL;
	free(p);
	free(m);
	return file;
}
static void bn_remove(const char *path) {
	wchar_t *p = bn_wide(path);
	if (p) {
		DeleteFileW(p);
		free(p);
	}
}
static bool bn_temp(char *path, size_t size) {
	wchar_t root[MAX_PATH], file[MAX_PATH];
	if (!GetTempPathW(MAX_PATH, root) || !GetTempFileNameW(root, L"apb", 0, file))
		return false;
	return WideCharToMultiByte(CP_UTF8, 0, file, -1, path, (int)size, NULL, NULL) > 0;
}
static char *bn_quote(const char *arg) {
	char *out = malloc(strlen(arg) * 2 + 3), *p = out;
	if (!out)
		return NULL;
	*p++ = '"';
	while (*arg) {
		int slashes = 0;
		while (*arg == '\\') {
			slashes++;
			arg++;
		}
		for (int i = 0; i < slashes * ((*arg == '"' || !*arg) ? 2 : 1); i++)
			*p++ = '\\';
		if (*arg == '"')
			*p++ = '\\';
		if (*arg)
			*p++ = *arg++;
	}
	*p++ = '"';
	*p   = 0;
	return out;
}
static int bn_process(char **argv, const char *log, bn_progress progress, void *user) {
	size_t size = 1;
	for (int i = 0; argv[i]; i++)
		size += strlen(argv[i]) * 2 + 4;
	char *command = calloc(size, 1);
	if (!command)
		return -1;
	for (int i = 0; argv[i]; i++) {
		char *q = bn_quote(argv[i]);
		if (!q) {
			free(command);
			return -1;
		}
		if (i)
			strcat(command, " ");
		strcat(command, q);
		free(q);
	}
	wchar_t *cmd = bn_wide(command), *exe = bn_wide(argv[0]), *file = bn_wide(log);
	free(command);
	SECURITY_ATTRIBUTES security = {.nLength = sizeof(security), .bInheritHandle = TRUE};
	HANDLE output = file ? CreateFileW(file, GENERIC_WRITE, FILE_SHARE_READ, &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) : INVALID_HANDLE_VALUE;
	STARTUPINFOW startup = {
	    .cb = sizeof(startup), .dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES, .wShowWindow = SW_HIDE, .hStdOutput = output, .hStdError = output};
	PROCESS_INFORMATION process = {0};
	bool ok = cmd && exe && output != INVALID_HANDLE_VALUE && CreateProcessW(exe, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup, &process);
	free(cmd);
	free(exe);
	free(file);
	if (output != INVALID_HANDLE_VALUE)
		CloseHandle(output);
	if (!ok)
		return -1;
	bool cancelled = false;
	while (WaitForSingleObject(process.hProcess, 100) == WAIT_TIMEOUT)
		if (progress && !progress(user, 0)) {
			cancelled = true;
			TerminateProcess(process.hProcess, 1);
			break;
		}
	WaitForSingleObject(process.hProcess, INFINITE);
	DWORD code = 1;
	GetExitCodeProcess(process.hProcess, &code);
	CloseHandle(process.hProcess);
	CloseHandle(process.hThread);
	return cancelled ? -2 : (int)code;
}
#elif defined(IRON_LINUX) || defined(IRON_MACOS)
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
static FILE  *bn_open(const char *path, const char *mode) {
	return fopen(path, mode);
}
static void bn_remove(const char *path) {
	unlink(path);
}
static bool bn_temp(char *path, size_t size) {
	snprintf(path, size, "/tmp/armorpaint-bake-XXXXXX");
	int fd = mkstemp(path);
	if (fd < 0)
		return false;
	close(fd);
	return true;
}
static int bn_process(char **argv, const char *log, bn_progress progress, void *user) {
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, log, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO);
	pid_t pid;
	int   error = posix_spawnp(&pid, argv[0], &actions, NULL, argv, environ);
	posix_spawn_file_actions_destroy(&actions);
	if (error)
		return -1;
	int status = 0;
	for (;;) {
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid)
			break;
		if (result < 0)
			return -1;
		if (progress && !progress(user, 0)) {
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			return -2;
		}
		usleep(100000);
	}
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif

static char *normal_bake_find_blender() {
#ifdef IRON_WINDOWS
	wchar_t path[1024];
	if (SearchPathW(NULL, L"blender.exe", NULL, 1024, path, NULL)) {
		char utf8[4096];
		WideCharToMultiByte(CP_UTF8, 0, path, -1, utf8, sizeof(utf8), NULL, NULL);
		return string_copy(utf8);
	}
	wchar_t root[1024], pattern[1200], best[1400] = L"";
	if (GetEnvironmentVariableW(L"ProgramFiles", root, 1024)) {
		swprintf(pattern, 1200, L"%ls\\Blender Foundation\\Blender *", root);
		WIN32_FIND_DATAW data;
		HANDLE           search = FindFirstFileW(pattern, &data);
		if (search != INVALID_HANDLE_VALUE) {
			do {
				swprintf(path, 1024, L"%ls\\Blender Foundation\\%ls\\blender.exe", root, data.cFileName);
				if (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES && wcscmp(path, best) > 0)
					wcscpy(best, path);
			} while (FindNextFileW(search, &data));
			FindClose(search);
		}
	}
	if (best[0]) {
		char utf8[4096];
		WideCharToMultiByte(CP_UTF8, 0, best, -1, utf8, sizeof(utf8), NULL, NULL);
		return string_copy(utf8);
	}
#elif defined(IRON_MACOS)
	if (iron_file_exists("/Applications/Blender.app/Contents/MacOS/Blender"))
		return string_copy("/Applications/Blender.app/Contents/MacOS/Blender");
#elif defined(IRON_LINUX)
	if (iron_file_exists("/usr/bin/blender"))
		return string_copy("/usr/bin/blender");
#endif
	return string_copy("");
}

static bool normal_bake_export_blend(normal_bake_session *s, char *output, size_t output_size, bool inspect) {
#if defined(IRON_WINDOWS) || defined(IRON_LINUX) || defined(IRON_MACOS)
	char script[4096], log[4200];
	if (!bn_temp(script, sizeof(script))) {
		snprintf(s->error, sizeof(s->error), "Could not create Blender temporary file");
		return false;
	}
	snprintf(output, output_size, "%s.obj", script);
	snprintf(log, sizeof(log), "%s.log", script);
	FILE *f       = bn_open(script, "wb");
	bool  written = f && fwrite(s->blender_script, 1, strlen(s->blender_script), f) == strlen(s->blender_script);
	if (f)
		fclose(f);
	char *argv[] = {s->blender,
	                "--background",
	                "--factory-startup",
	                "--disable-autoexec",
	                s->path,
	                "--python-exit-code",
	                "1",
	                "--python",
	                script,
	                "--",
	                output,
	                s->blender_object,
	                s->blender_selected ? "selected" : "visible",
	                inspect ? "inspect" : "export",
	                NULL};
	int   code   = written ? bn_process(argv, log, normal_bake_progress, s) : -1;
	if (code) {
		snprintf(s->error, sizeof(s->error), code == -2 ? "Blender import cancelled" : "Blender export failed. Check executable and source objects.");
		f = bn_open(log, "rb");
		if (f) {
			char line[512];
			while (fgets(line, sizeof(line), f))
				if (strncmp(line, "RuntimeError: ", 14) == 0)
					snprintf(s->error, sizeof(s->error), "%.*s", 230, line + 14);
			fclose(f);
		}
	}
	bn_remove(script);
	bn_remove(log);
	if (code) {
		bn_remove(output);
		output[0] = 0;
		return false;
	}
	return true;
#else
	snprintf(s->error, sizeof(s->error), "Blender import requires a desktop build");
	return false;
#endif
}
