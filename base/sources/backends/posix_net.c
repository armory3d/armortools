#include <iron_net.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <openssl/ssl.h>

static SSL_CTX *ctx = NULL;
static char *buf = NULL;
static int buf_len = 1024 * 1024;

void iron_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
					   iron_http_callback_t callback, void *callbackdata) {

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct hostent *server = gethostbyname(url);
	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	memmove(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
	server_addr.sin_port = htons(port);

	if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		close(sock_fd);
		callback(1, 0, NULL, callbackdata);
		return;
	}

	if (ctx == NULL) {
		ctx = SSL_CTX_new(TLS_client_method());
	}

	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sock_fd);

	if (SSL_connect(ssl) <= 0) {
		SSL_free(ssl);
		close(sock_fd);
		callback(1, 0, NULL, callbackdata);
		return;
	}

	// For HTTP/1.1, implement "transfer-encoding: chunked"
	char request[1024];
	int request_len = snprintf(request, sizeof(request), "GET /%s HTTP/1.0\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, url, port);
	SSL_write(ssl, request, request_len);

	// Read
	if (buf == NULL) {
		buf = malloc(buf_len);
	}
	int pos = 0;
	int n;
	while ((n = SSL_read(ssl, buf + pos, buf_len - pos - 1)) > 0) {
		pos += n;
		if (pos == buf_len - 1) {
			buf_len *= 2;
			buf = realloc(buf, buf_len);
		}
	}

	// Parse
	int status_code = 0;
	const char *body_ptr = "";
	char *headers_end = strstr(buf, "\r\n\r\n");
	body_ptr = headers_end + 4;
	callback(0, status_code, body_ptr, callbackdata);

	SSL_free(ssl);
	close(sock_fd);
}
