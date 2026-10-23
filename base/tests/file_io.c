// Windows regression tests using the production file code and a controlled HTTP transport.
#include <stdio.h>
#include <stdbool.h>
static bool fail_write;
static size_t checked_fwrite(const void *bytes, size_t size, size_t count, FILE *file) {
	return fail_write ? 0 : fwrite(bytes, size, count, file);
}
#define fwrite checked_fwrite
#include "../sources/iron_file.c"
#undef fwrite
#include <assert.h>

char temp_string[1024 * 128];
wchar_t temp_wstring[1024 * 32];
const char *iron_internal_save_path(void) { return "./"; }

// Unused application services fail loudly if a test unexpectedly reaches them.
void iron_log(const char *format, ...) { abort(); }
int iron_sys_command(char *cmd) { abort(); }
directory open_dir(const char *path) { abort(); }
file read_next_file(directory *dir) { abort(); }
void close_dir(directory *dir) { abort(); }
char *path_join(char *a, char *b) { abort(); }
bool path_is_protected(void) { abort(); }
bool path_is_folder(char *p) { abort(); }
char *strings_check_internet_connection(void) { abort(); }
void console_error(char *s) { abort(); }
char *sys_buffer_to_string(buffer_t *b) { abort(); }

static iron_https_callback_t response;
static void *response_data;
static char *download_temp;
static int completed;
static bool succeeded;

void iron_net_request(const char *host, const char *path, const char *data, int port, int method, const char *headers,
                      iron_https_callback_t callback, void *context, const char *destination) {
	assert(strcmp(host, "example.test") == 0);
	assert(strcmp(path, "file") == 0);
	response = callback;
	response_data = context;
	download_temp = string_copy((char *)destination);
}

static void done(char *url, bool success) {
	++completed;
	succeeded = success;
}

static void expect_contents(char *path, char *expected) {
	iron_file_reader_t reader;
	assert(iron_file_reader_open(&reader, path, IRON_FILE_TYPE_ASSET));
	assert(reader.size == strlen(expected));
	char bytes[64] = {0};
	assert(iron_file_reader_read(&reader, bytes, reader.size) == reader.size);
	assert(strcmp(bytes, expected) == 0);
	iron_file_reader_close(&reader);
}

static void finish(int status, const char *bytes) {
	if (bytes != NULL) {
		FILE *f = file_open(download_temp, "wb");
		assert(f != NULL);
		assert(fwrite(bytes, 1, strlen(bytes), f) == strlen(bytes));
		assert(fclose(f) == 0);
	}
	int before = completed;
	response(status, NULL, response_data);
	assert(completed == before + 1);
	assert(file_open(download_temp, "rb") == NULL);
	free(download_temp);
	download_temp = NULL;
}

int main(int argc, char **argv) {
	assert(argc == 2); // Existing, isolated test directory, supplied as an absolute path.
	char *target = string("%s/project.arm", argv[1]);
	buffer_t old = {.buffer = (u8 *)"original", .length = 8};
	buffer_t replacement = {.buffer = (u8 *)"replacement", .length = 11};
	iron_internal_set_files_location("deliberately-missing-assets");
	assert(iron_file_save_bytes(target, &old, 0));
	expect_contents(target, "original"); // Also exercises forward-slash Windows absolute paths.
	fail_write = true;
	assert(!iron_file_save_bytes(target, &replacement, 0));
	fail_write = false;
	expect_contents(target, "original");
	wchar_t *wide = file_wide_path(target);
	HANDLE lock = CreateFileW(wide, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(lock != INVALID_HANDLE_VALUE);
	assert(!iron_file_save_bytes(target, &replacement, 0));
	expect_contents(target, "original");
	CloseHandle(lock);
	free(wide);
	assert(iron_file_save_bytes(target, &replacement, 0));
	expect_contents(target, "replacement");
	assert(!iron_file_save_bytes(string("%s/missing/project.arm", argv[1]), &old, 0));
	assert(!iron_file_save_bytes(NULL, &old, 0));
	assert(!iron_file_save_bytes(target, NULL, 0));
	char *unicode = string("%s/gr\xc3\xbc\xc3\x9f\x65.arm", argv[1]);
	assert(iron_file_save_bytes(unicode, &old, 0));
	expect_contents(unicode, "original");

	char *url = "https://example.test/file";
	file_download_to(url, target, done, 0);
	finish(404, "error page");
	assert(!succeeded);
	expect_contents(target, "replacement");
	file_download_to(url, target, done, 12);
	finish(200, "partial");
	assert(!succeeded);
	expect_contents(target, "replacement");
	file_download_to(url, target, done, 0);
	finish(0, "interrupted");
	assert(!succeeded);
	expect_contents(target, "replacement");
	file_download_to(url, target, done, 0);
	finish(200, "");
	assert(!succeeded);
	file_download_to(url, target, done, 0);
	finish(200, NULL);
	assert(!succeeded);
	file_download_to(url, target, done, 8);
	finish(200, "complete");
	assert(succeeded);
	expect_contents(target, "complete");
	char *fresh = string("%s/new-download", argv[1]);
	file_download_to(url, fresh, done, 0);
	finish(503, "unavailable");
	assert(!succeeded && file_open(fresh, "rb") == NULL);
	file_download_to(url, fresh, done, 0);
	finish(200, "retry");
	assert(succeeded);
	expect_contents(fresh, "retry");
	int before = completed;
	file_download_to(url, string("%s/missing/download", argv[1]), done, 0);
	assert(completed == before + 1 && !succeeded);
	file_download_to("http://example.test/file", target, done, 0);
	assert(completed == before + 2 && !succeeded);
	expect_contents(target, "complete");
	puts("PASS: atomic save, locked destination, missing directory, Unicode, absolute paths, HTTP failures, incomplete downloads and retry");
	return 0;
}
