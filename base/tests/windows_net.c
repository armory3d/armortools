// Exercise production WinHTTP completion/error paths without a remote service.
#include "../sources/backends/windows_net.c"
#include <assert.h>

static int callbacks;
static int last_status;
static void completed(int status, const char *body, void *data) {
	++callbacks;
	last_status = status;
}

static async_context_t *context(void) {
	async_context_t *c = calloc(1, sizeof(*c));
	c->hfile = INVALID_HANDLE_VALUE;
	c->callback = completed;
	c->status = 200;
	return c;
}

static void check(async_context_t *c, int expected) {
	int before = callbacks;
	iron_net_update();
	assert(callbacks == before + 1 && last_status == expected);
	assert(pending_head == NULL);
	// Real WinHTTP requests free this in HANDLE_CLOSING; these contexts have no HTTP handles.
	free(c);
}

int main(void) {
	InitializeCriticalSection(&cs);
	initialized = TRUE;
	async_context_t *c = context();
	c->has_content_length = TRUE;
	c->content_length = 10;
	c->bytes_received = 3;
	finish_request(c, TRUE);
	finish_request(c, FALSE); // A late error must not dispatch twice.
	check(c, 0);

	c = context();
	c->has_content_length = TRUE;
	c->content_length = 10;
	c->bytes_received = 10;
	finish_request(c, TRUE);
	check(c, 200);

	c = context();
	FILE *file = fopen("write-failure.tmp", "wb");
	assert(file != NULL && fclose(file) == 0);
	c->hfile = CreateFileW(L"write-failure.tmp", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(c->hfile != INVALID_HANDLE_VALUE);
	c->file_buffer = "test";
	iron_winhttp_callback(NULL, (DWORD_PTR)c, WINHTTP_CALLBACK_STATUS_READ_COMPLETE, NULL, 4);
	assert(c->hfile == INVALID_HANDLE_VALUE);
	check(c, 0);
	assert(remove("write-failure.tmp") == 0);

	c = context();
	iron_winhttp_callback(NULL, (DWORD_PTR)c, WINHTTP_CALLBACK_STATUS_REQUEST_ERROR, NULL, 0);
	check(c, 0);
	DeleteCriticalSection(&cs);
	puts("PASS: WinHTTP truncated response, successful completion, write failure and single error callback");
	return 0;
}
