#pragma once

#include <iron_global.h>
#include <stdbool.h>

#define IRON_HTTP_GET 0
#define IRON_HTTP_POST 1
#define IRON_HTTP_PUT 2
#define IRON_HTTP_DELETE 3

typedef void (*iron_http_callback_t)(int error, int response, const char *body, void *callbackdata);

void iron_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                                 iron_http_callback_t callback, void *callbackdata);

// typedef enum iron_socket_protocol {
// 	IRON_SOCKET_PROTOCOL_UDP,
// 	IRON_SOCKET_PROTOCOL_TCP
// } iron_socket_protocol_t;

// typedef enum iron_socket_family {
// 	IRON_SOCKET_FAMILY_IP4,
// 	IRON_SOCKET_FAMILY_IP6
// } iron_socket_family_t;

// #ifdef IRON_WINDOWS
// typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
// typedef UINT_PTR SOCKET;
// #endif

// typedef struct iron_socket {
// #ifdef IRON_WINDOWS
// 	SOCKET handle;
// #else
// 	int handle;
// #endif
// 	uint32_t host;
// 	uint32_t port;
// 	iron_socket_protocol_t protocol;
// 	iron_socket_family_t family;
// 	bool connected;
// } iron_socket_t;

// typedef struct iron_socket_options {
// 	bool non_blocking;
// 	bool broadcast;
// 	bool tcp_no_delay;
// } iron_socket_options_t;

// void iron_socket_options_set_defaults(iron_socket_options_t *options);
// void iron_socket_init(iron_socket_t *socket);
// bool iron_socket_set(iron_socket_t *socket, const char *host, int port, iron_socket_family_t family, iron_socket_protocol_t protocol);
// void iron_socket_destroy(iron_socket_t *socket);
// bool iron_socket_open(iron_socket_t *socket, iron_socket_options_t *options);
// bool iron_socket_select(iron_socket_t *socket, uint32_t waittime, bool read, bool write);

// /*Typically these are server actions.*/
// bool iron_socket_bind(iron_socket_t *socket);
// bool iron_socket_listen(iron_socket_t *socket, int backlog);
// bool iron_socket_accept(iron_socket_t *socket, iron_socket_t *new_socket, unsigned *remote_address, unsigned *remote_port);

// /*Typically this is a client action.*/
// bool iron_socket_connect(iron_socket_t *socket);

// int iron_socket_send(iron_socket_t *socket, const uint8_t *data, int size);
// int iron_socket_send_address(iron_socket_t *socket, unsigned address, int port, const uint8_t *data, int size);
// int iron_socket_send_url(iron_socket_t *socket, const char *url, int port, const uint8_t *data, int size);
// int iron_socket_receive(iron_socket_t *socket, uint8_t *data, int maxSize, unsigned *from_address, unsigned *from_port);

// unsigned iron_url_to_int(const char *url, int port);

// #ifdef IRON_IMPLEMENTATION_ROOT
// #define IRON_IMPLEMENTATION
// #endif

// #ifdef IRON_IMPLEMENTATION

// #undef IRON_IMPLEMENTATION
// #include <stb_sprintf.h>
// #include <iron_system.h>
// #define IRON_IMPLEMENTATION

// #include <errno.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include <string.h>

// #if defined(IRON_WINDOWS)

// // Windows 7
// #define WINVER 0x0601
// #define _WIN32_WINNT 0x0601

// #define NOATOM
// #define NOCLIPBOARD
// #define NOCOLOR
// #define NOCOMM
// #define NOCTLMGR
// #define NODEFERWINDOWPOS
// #define NODRAWTEXT
// #define NOGDI
// #define NOGDICAPMASKS
// #define NOHELP
// #define NOICONS
// #define NOKANJI
// #define NOKEYSTATES
// #define NOMB
// #define NOMCX
// #define NOMEMMGR
// #define NOMENUS
// #define NOMETAFILE
// #define NOMINMAX
// #define NOMSG
// #define NONLS
// #define NOOPENFILE
// #define NOPROFILER
// #define NORASTEROPS
// #define NOSCROLL
// #define NOSERVICE
// #define NOSHOWWINDOW
// #define NOSOUND
// #define NOSYSCOMMANDS
// #define NOSYSMETRICS
// #define NOTEXTMETRIC
// #define NOUSER
// #define NOVIRTUALKEYCODES
// #define NOWH
// #define NOWINMESSAGES
// #define NOWINOFFSETS
// #define NOWINSTYLES
// #define WIN32_LEAN_AND_MEAN

// #include <Ws2tcpip.h>
// #include <winsock2.h>
// #elif defined(IRON_POSIX)
// #include <arpa/inet.h> // for inet_addr()
// #include <ctype.h>
// #include <fcntl.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <unistd.h>
// #endif

// #if defined(IRON_POSIX)
// #include <sys/select.h>
// #endif

// static int counter = 0;

// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// // Important: Must be cleaned with freeaddrinfo(address) later if the result is 0 in order to prevent memory leaks
// static int resolveAddress(const char *url, int port, struct addrinfo **result) {
// 	struct addrinfo hints = {0};
// 	hints.ai_family = AF_INET;
// 	hints.ai_socktype = SOCK_DGRAM;
// 	hints.ai_protocol = IPPROTO_UDP;

// 	char serv[6];
// 	sprintf(serv, "%u", port);

// 	return getaddrinfo(url, serv, &hints, result);
// }
// #endif

// bool iron_socket_bind(iron_socket_t *sock) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	struct sockaddr_in address;
// 	address.sin_family = sock->family == IRON_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6;
// 	address.sin_addr.s_addr = sock->host;
// 	address.sin_port = sock->port;
// 	if (bind(sock->handle, (const struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
// 		iron_error("Could not bind socket: %s", strerror(errno));
// 		return false;
// 	}
// 	return true;
// #else
// 	return false;
// #endif
// }

// void iron_socket_options_set_defaults(iron_socket_options_t *options) {
// 	options->non_blocking = true;
// 	options->broadcast = false;
// 	options->tcp_no_delay = false;
// }

// void iron_socket_init(iron_socket_t *sock) {
// 	sock->handle = 0;

// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	sock->host = INADDR_ANY;
// 	sock->port = htons((unsigned short)8080);
// 	sock->protocol = IRON_SOCKET_PROTOCOL_TCP;
// 	sock->family = IRON_SOCKET_FAMILY_IP4;
// #endif
// 	sock->connected = false;

// #if defined(IRON_WINDOWS)
// 	if (counter == 0) {
// 		WSADATA WsaData;
// 		WSAStartup(MAKEWORD(2, 2), &WsaData);
// 	}
// #endif
// 	++counter;
// }

// bool iron_socket_open(iron_socket_t *sock, struct iron_socket_options *options) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	switch (sock->protocol) {
// 	case IRON_SOCKET_PROTOCOL_UDP:
// 		sock->handle = socket(sock->family == IRON_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
// 		break;
// 	case IRON_SOCKET_PROTOCOL_TCP:
// 		sock->handle = socket(sock->family == IRON_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);
// 		break;
// 	default:
// 		iron_error("Unsupported socket protocol.");
// 		return false;
// 	}

// 	if (sock->handle <= 0) {
// 		iron_error("Could not create socket.");
// #if defined(IRON_WINDOWS)
// 		int errorCode = WSAGetLastError();
// 		switch (errorCode) {
// 		case (WSANOTINITIALISED):
// 			iron_error("A successful WSAStartup call must occur before using this function.");
// 			break;
// 		case (WSAENETDOWN):
// 			iron_error("The network subsystem or the associated service provider has failed.");
// 			break;
// 		case (WSAEAFNOSUPPORT):
// 			iron_error(
// 			         "The specified address family is not supported.For example, an application tried to create a socket for the AF_IRDA address "
// 			         "family but an infrared adapter and device driver is not installed on the local computer.");
// 			break;
// 		case (WSAEINPROGRESS):
// 			iron_error(
// 			         "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
// 			break;
// 		case (WSAEMFILE):
// 			iron_error("No more socket descriptors are available.");
// 			break;
// 		case (WSAEINVAL):
// 			iron_error(
// 			         "An invalid argument was supplied.This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol "
// 			         "parameter are unspecified.");
// 			break;
// 		case (WSAENOBUFS):
// 			iron_error("No buffer space is available.The socket cannot be created.");
// 			break;
// 		case (WSAEPROTONOSUPPORT):
// 			iron_error("The specified protocol is not supported.");
// 			break;
// 		case (WSAEPROTOTYPE):
// 			iron_error("The specified protocol is the wrong type for this socket.");
// 			break;
// 		case (WSAEPROVIDERFAILEDINIT):
// 			iron_error(
// 			         "The service provider failed to initialize.This error is returned if a layered service provider(LSP) or namespace provider was "
// 			         "improperly installed or the provider fails to operate correctly.");
// 			break;
// 		case (WSAESOCKTNOSUPPORT):
// 			iron_error("The specified socket type is not supported in this address family.");
// 			break;
// 		case (WSAEINVALIDPROVIDER):
// 			iron_error("The service provider returned a version other than 2.2.");
// 			break;
// 		case (WSAEINVALIDPROCTABLE):
// 			iron_error("The service provider returned an invalid or incomplete procedure table to the WSPStartup.");
// 			break;
// 		default:
// 			iron_error("Unknown error.");
// 		}
// #elif defined(IRON_POSIX)
// 		iron_error("%s", strerror(errno));
// #endif
// 		return false;
// 	}
// #endif

// 	if (options) {
// 		if (options->non_blocking) {
// #if defined(IRON_WINDOWS)
// 			DWORD value = 1;
// 			if (ioctlsocket(sock->handle, FIONBIO, &value) != 0) {
// 				iron_error("Could not set non-blocking mode.");
// 				return false;
// 			}
// #elif defined(IRON_POSIX)
// 			int value = 1;
// 			if (fcntl(sock->handle, F_SETFL, O_NONBLOCK, value) == -1) {
// 				iron_error("Could not set non-blocking mode.");
// 				return false;
// 			}
// #endif
// 		}

// 		if (options->broadcast) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 			int value = 1;
// 			if (setsockopt(sock->handle, SOL_SOCKET, SO_BROADCAST, (const char *)&value, sizeof(value)) < 0) {
// 				iron_error("Could not set broadcast mode.");
// 				return false;
// 			}
// #endif
// 		}

// 		if (options->tcp_no_delay) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 			int value = 1;
// 			if (setsockopt(sock->handle, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof(value)) != 0) {
// 				iron_error("Could not set no-delay mode.");
// 				return false;
// 			}
// #endif
// 		}
// 	}

// 	return true;
// }

// void iron_socket_destroy(iron_socket_t *sock) {
// #if defined(IRON_WINDOWS)
// 	closesocket(sock->handle);
// #elif defined(IRON_POSIX)
// 	close(sock->handle);
// #endif

// 	memset(sock, 0, sizeof(iron_socket_t));

// 	--counter;
// #if defined(IRON_WINDOWS)
// 	if (counter == 0) {
// 		WSACleanup();
// 	}
// #endif
// }

// bool iron_socket_select(iron_socket_t *sock, uint32_t waittime, bool read, bool write) {
// #if (defined(IRON_WINDOWS) || defined(IRON_POSIX))
// 	fd_set r_fds, w_fds;
// 	struct timeval timeout;

// 	FD_ZERO(&r_fds);
// 	FD_ZERO(&w_fds);

// 	FD_SET(sock->handle, &r_fds);
// 	FD_SET(sock->handle, &w_fds);

// 	timeout.tv_sec = waittime;
// 	timeout.tv_usec = 0;

// 	if (select(0, &r_fds, &w_fds, NULL, &timeout) < 0) {
// 		iron_error("iron_socket_select didn't work: %s", strerror(errno));
// 		return false;
// 	}

// 	if (read && write) {
// 		return FD_ISSET(sock->handle, &w_fds) && FD_ISSET(sock->handle, &r_fds);
// 	}
// 	else if (read) {
// 		return FD_ISSET(sock->handle, &r_fds);
// 	}
// 	else if (write) {
// 		return FD_ISSET(sock->handle, &w_fds);
// 	}
// 	else {
// 		iron_error("Calling iron_socket_select with both read and write set to false is useless.");
// 		return false;
// 	}
// #else
// 	return false;
// #endif
// }

// bool iron_socket_set(iron_socket_t *sock, const char *host, int port, iron_socket_family_t family, iron_socket_protocol_t protocol) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)

// 	sock->family = family;
// 	sock->protocol = protocol;
// 	sock->port = htons((unsigned short)port);

// 	if (host == NULL)
// 		return true;

// 	if (isdigit(host[0]) || (family == IRON_SOCKET_FAMILY_IP6 && host[4] == ':')) { // Is IPv4 or IPv6 string
// 		struct in_addr addr;

// 		if (inet_pton(sock->family == IRON_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6, host, &addr) == 0) {
// 			iron_error("Invalid %s address: %s\n", sock->family == IRON_SOCKET_FAMILY_IP4 ? "IPv4" : "IPv6", host);
// 			return false;
// 		}
// 		sock->host = addr.s_addr;
// 		return true;
// 	}
// 	else {
// 		struct addrinfo *address = NULL;
// 		int res = resolveAddress(host, port, &address);
// 		if (res != 0) {
// 			iron_error("Could not resolve address.");
// 			return false;
// 		}
// #if defined(IRON_POSIX)
// 		sock->host = ((struct sockaddr_in *)address->ai_addr)->sin_addr.s_addr;
// #else
// 		sock->host = ((struct sockaddr_in *)address->ai_addr)->sin_addr.S_un.S_addr;
// #endif
// 		freeaddrinfo(address);

// 		return true;
// 	}
// #else
// 	return false;
// #endif
// }

// bool iron_socket_listen(iron_socket_t *socket, int backlog) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	int res = listen(socket->handle, backlog);
// 	return (res == 0);
// #else
// 	return false;
// #endif
// }

// bool iron_socket_accept(iron_socket_t *sock, iron_socket_t *newSocket, unsigned *remoteAddress, unsigned *remotePort) {
// #if defined(IRON_WINDOWS)
// 	typedef int socklen_t;
// #endif
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	struct sockaddr_in addr;
// 	socklen_t addrLength = sizeof(addr);
// 	newSocket->handle = accept(sock->handle, (struct sockaddr *)&addr, &addrLength);
// 	if (newSocket->handle <= 0) {
// 		return false;
// 	}

// 	newSocket->connected = sock->connected = true;
// 	newSocket->host = addr.sin_addr.s_addr;
// 	newSocket->port = addr.sin_port;
// 	newSocket->family = sock->family;
// 	newSocket->protocol = sock->protocol;
// 	*remoteAddress = ntohl(addr.sin_addr.s_addr);
// 	*remotePort = ntohs(addr.sin_port);
// 	return true;
// #else
// 	return false;
// #endif
// }

// bool iron_socket_connect(iron_socket_t *sock) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	struct sockaddr_in addr;
// 	addr.sin_family = sock->family == IRON_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6;
// 	addr.sin_addr.s_addr = sock->host;
// 	addr.sin_port = sock->port;

// 	int res = connect(sock->handle, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
// 	return sock->connected = (res == 0);
// #else
// 	return false;
// #endif
// }

// int iron_socket_send(iron_socket_t *sock, const uint8_t *data, int size) {
// #if defined(IRON_WINDOWS)
// 	typedef int socklen_t;
// #endif
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	if (sock->protocol == IRON_SOCKET_PROTOCOL_UDP) {
// 		struct sockaddr_in addr;
// 		addr.sin_family = sock->family == IRON_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6;
// 		addr.sin_addr.s_addr = sock->host;
// 		addr.sin_port = sock->port;

// 		size_t sent = sendto(sock->handle, (const char *)data, size, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
// 		if (sent != size) {
// 			iron_error("Could not send packet.");
// 			return -1;
// 		}
// 		return (int)sent;
// 	}
// 	else {
// 		if (!sock->connected) {
// 			iron_error("Call iron_sockect_connect/bind before send/recv can be called for TCP sockets.");
// 			return -1;
// 		}

// 		size_t sent = send(sock->handle, (const char *)data, size, 0);
// 		if (sent != size) {
// 			iron_error("Could not send packet.");
// 		}
// 		return (int)sent;
// 	}
// #else
// 	return 0;
// #endif
// }

// int iron_socket_send_address(iron_socket_t *sock, unsigned address, int port, const uint8_t *data, int size) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	struct sockaddr_in addr;
// 	addr.sin_family = AF_INET;
// 	addr.sin_addr.s_addr = htonl(address);
// 	addr.sin_port = htons(port);

// 	size_t sent = sendto(sock->handle, (const char *)data, size, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
// 	if (sent != size) {
// 		iron_error("Could not send packet.");
// 	}
// 	return (int)sent;
// #else
// 	return 0;
// #endif
// }

// int iron_socket_send_url(iron_socket_t *sock, const char *url, int port, const uint8_t *data, int size) {
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)
// 	struct addrinfo *address = NULL;
// 	int res = resolveAddress(url, port, &address);
// 	if (res != 0) {
// 		iron_error("Could not resolve address.");
// 		return 0;
// 	}

// 	size_t sent = sendto(sock->handle, (const char *)data, size, 0, address->ai_addr, sizeof(struct sockaddr_in));
// 	if (sent != size) {
// 		iron_error("Could not send packet.");
// 	}
// 	freeaddrinfo(address);
// 	return (int)sent;
// #else
// 	return 0;
// #endif
// }

// int iron_socket_receive(iron_socket_t *sock, uint8_t *data, int maxSize, unsigned *fromAddress, unsigned *fromPort) {
// #if defined(IRON_WINDOWS)
// 	typedef int socklen_t;
// 	typedef int ssize_t;
// #endif
// #if defined(IRON_WINDOWS) || defined(IRON_POSIX)

// 	if (sock->protocol == IRON_SOCKET_PROTOCOL_UDP) {
// 		struct sockaddr_in from;
// 		socklen_t fromLength = sizeof(from);
// 		ssize_t bytes = recvfrom(sock->handle, (char *)data, maxSize, 0, (struct sockaddr *)&from, &fromLength);
// 		if (bytes <= 0) {
// 			return (int)bytes;
// 		}
// 		*fromAddress = ntohl(from.sin_addr.s_addr);
// 		*fromPort = ntohs(from.sin_port);
// 		return (int)bytes;
// 	}
// 	else {

// 		if (!sock->connected) {
// 			iron_error("Call iron_sockect_connect/bind before send/recv can be called for TCP sockets.");
// 			return -1;
// 		}
// 		ssize_t bytes = recv(sock->handle, (char *)data, maxSize, 0);
// 		*fromAddress = ntohl(sock->host);
// 		*fromPort = ntohs(sock->port);
// 		return (int)bytes;
// 	}
// #else
// 	return 0;
// #endif
// }

// unsigned iron_url_to_int(const char *url, int port) {
// #if defined(IRON_WINDOWS)
// 	struct addrinfo *address = NULL;
// 	int res = resolveAddress(url, port, &address);
// 	if (res != 0) {
// 		iron_error("Could not resolve address.");
// 		return -1;
// 	}

// 	unsigned fromAddress = ntohl(((struct sockaddr_in *)address->ai_addr)->sin_addr.S_un.S_addr);
// 	freeaddrinfo(address);

// 	return fromAddress;
// #else
// 	return 0;
// #endif
// }

// #endif
