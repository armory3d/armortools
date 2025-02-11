#pragma once

#include <kinc/global.h>

/*! \file socket.h
    \brief Provides low-level network-communication via UDP or TCP-sockets.
*/

typedef enum kinc_socket_protocol {
	KINC_SOCKET_PROTOCOL_UDP,
	KINC_SOCKET_PROTOCOL_TCP
} kinc_socket_protocol_t;

typedef enum kinc_socket_family {
	KINC_SOCKET_FAMILY_IP4,
	KINC_SOCKET_FAMILY_IP6
} kinc_socket_family_t;

#ifdef KINC_MICROSOFT
#if defined(_WIN64)
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
#else
#if !defined _W64
#define _W64
#endif
typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;
#endif
typedef UINT_PTR SOCKET;
#endif

typedef struct kinc_socket {
#ifdef KINC_MICROSOFT
	SOCKET handle;
#else
	int handle;
#endif
	uint32_t host;
	uint32_t port;
	kinc_socket_protocol_t protocol;
	kinc_socket_family_t family;
	bool connected;
} kinc_socket_t;

typedef struct kinc_socket_options {
	bool non_blocking;
	bool broadcast;
	bool tcp_no_delay;
} kinc_socket_options_t;

void kinc_socket_options_set_defaults(kinc_socket_options_t *options);
void kinc_socket_init(kinc_socket_t *socket);
bool kinc_socket_set(kinc_socket_t *socket, const char *host, int port, kinc_socket_family_t family, kinc_socket_protocol_t protocol);
void kinc_socket_destroy(kinc_socket_t *socket);
bool kinc_socket_open(kinc_socket_t *socket, kinc_socket_options_t *options);
bool kinc_socket_select(kinc_socket_t *socket, uint32_t waittime, bool read, bool write);

/*Typically these are server actions.*/
bool kinc_socket_bind(kinc_socket_t *socket);
bool kinc_socket_listen(kinc_socket_t *socket, int backlog);
bool kinc_socket_accept(kinc_socket_t *socket, kinc_socket_t *new_socket, unsigned *remote_address, unsigned *remote_port);

/*Typically this is a client action.*/
bool kinc_socket_connect(kinc_socket_t *socket);

int kinc_socket_send(kinc_socket_t *socket, const uint8_t *data, int size);
int kinc_socket_send_address(kinc_socket_t *socket, unsigned address, int port, const uint8_t *data, int size);
int kinc_socket_send_url(kinc_socket_t *socket, const char *url, int port, const uint8_t *data, int size);
int kinc_socket_receive(kinc_socket_t *socket, uint8_t *data, int maxSize, unsigned *from_address, unsigned *from_port);

unsigned kinc_url_to_int(const char *url, int port);

#ifdef KINC_IMPLEMENTATION_NETWORK
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#undef KINC_IMPLEMENTATION
#include <stb_sprintf.h>
#include <kinc/log.h>
#define KINC_IMPLEMENTATION

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(KINC_WINDOWS)

// Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <Ws2tcpip.h>
#include <winsock2.h>
#elif defined(KINC_POSIX)
#include <arpa/inet.h> // for inet_addr()
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#if defined(KINC_POSIX)
#include <sys/select.h>
#endif

static int counter = 0;

#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
// Important: Must be cleaned with freeaddrinfo(address) later if the result is 0 in order to prevent memory leaks
static int resolveAddress(const char *url, int port, struct addrinfo **result) {
	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	char serv[6];
	sprintf(serv, "%u", port);

	return getaddrinfo(url, serv, &hints, result);
}
#endif

bool kinc_socket_bind(kinc_socket_t *sock) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	struct sockaddr_in address;
	address.sin_family = sock->family == KINC_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6;
	address.sin_addr.s_addr = sock->host;
	address.sin_port = sock->port;
	if (bind(sock->handle, (const struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not bind socket: %s", strerror(errno));
		return false;
	}
	return true;
#else
	return false;
#endif
}

void kinc_socket_options_set_defaults(kinc_socket_options_t *options) {
	options->non_blocking = true;
	options->broadcast = false;
	options->tcp_no_delay = false;
}

void kinc_socket_init(kinc_socket_t *sock) {
	sock->handle = 0;

#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	sock->host = INADDR_ANY;
	sock->port = htons((unsigned short)8080);
	sock->protocol = KINC_SOCKET_PROTOCOL_TCP;
	sock->family = KINC_SOCKET_FAMILY_IP4;
#endif
	sock->connected = false;

#if defined(KINC_WINDOWS)
	if (counter == 0) {
		WSADATA WsaData;
		WSAStartup(MAKEWORD(2, 2), &WsaData);
	}
#endif
	++counter;
}

bool kinc_socket_open(kinc_socket_t *sock, struct kinc_socket_options *options) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	switch (sock->protocol) {
	case KINC_SOCKET_PROTOCOL_UDP:
		sock->handle = socket(sock->family == KINC_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		break;
	case KINC_SOCKET_PROTOCOL_TCP:
		sock->handle = socket(sock->family == KINC_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		break;
	default:
		kinc_log(KINC_LOG_LEVEL_ERROR, "Unsupported socket protocol.");
		return false;
	}

	if (sock->handle <= 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not create socket.");
#if defined(KINC_WINDOWS)
		int errorCode = WSAGetLastError();
		switch (errorCode) {
		case (WSANOTINITIALISED):
			kinc_log(KINC_LOG_LEVEL_ERROR, "A successful WSAStartup call must occur before using this function.");
			break;
		case (WSAENETDOWN):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The network subsystem or the associated service provider has failed.");
			break;
		case (WSAEAFNOSUPPORT):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			         "The specified address family is not supported.For example, an application tried to create a socket for the AF_IRDA address "
			         "family but an infrared adapter and device driver is not installed on the local computer.");
			break;
		case (WSAEINPROGRESS):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			         "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
			break;
		case (WSAEMFILE):
			kinc_log(KINC_LOG_LEVEL_ERROR, "No more socket descriptors are available.");
			break;
		case (WSAEINVAL):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			         "An invalid argument was supplied.This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol "
			         "parameter are unspecified.");
			break;
		case (WSAENOBUFS):
			kinc_log(KINC_LOG_LEVEL_ERROR, "No buffer space is available.The socket cannot be created.");
			break;
		case (WSAEPROTONOSUPPORT):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The specified protocol is not supported.");
			break;
		case (WSAEPROTOTYPE):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The specified protocol is the wrong type for this socket.");
			break;
		case (WSAEPROVIDERFAILEDINIT):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			         "The service provider failed to initialize.This error is returned if a layered service provider(LSP) or namespace provider was "
			         "improperly installed or the provider fails to operate correctly.");
			break;
		case (WSAESOCKTNOSUPPORT):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The specified socket type is not supported in this address family.");
			break;
		case (WSAEINVALIDPROVIDER):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The service provider returned a version other than 2.2.");
			break;
		case (WSAEINVALIDPROCTABLE):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The service provider returned an invalid or incomplete procedure table to the WSPStartup.");
			break;
		default:
			kinc_log(KINC_LOG_LEVEL_ERROR, "Unknown error.");
		}
#elif defined(KINC_POSIX)
		kinc_log(KINC_LOG_LEVEL_ERROR, "%s", strerror(errno));
#endif
		return false;
	}
#endif

	if (options) {
		if (options->non_blocking) {
#if defined(KINC_WINDOWS)
			DWORD value = 1;
			if (ioctlsocket(sock->handle, FIONBIO, &value) != 0) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set non-blocking mode.");
				return false;
			}
#elif defined(KINC_POSIX)
			int value = 1;
			if (fcntl(sock->handle, F_SETFL, O_NONBLOCK, value) == -1) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set non-blocking mode.");
				return false;
			}
#endif
		}

		if (options->broadcast) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
			int value = 1;
			if (setsockopt(sock->handle, SOL_SOCKET, SO_BROADCAST, (const char *)&value, sizeof(value)) < 0) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set broadcast mode.");
				return false;
			}
#endif
		}

		if (options->tcp_no_delay) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
			int value = 1;
			if (setsockopt(sock->handle, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof(value)) != 0) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set no-delay mode.");
				return false;
			}
#endif
		}
	}

	return true;
}

void kinc_socket_destroy(kinc_socket_t *sock) {
#if defined(KINC_WINDOWS)
	closesocket(sock->handle);
#elif defined(KINC_POSIX)
	close(sock->handle);
#endif

	memset(sock, 0, sizeof(kinc_socket_t));

	--counter;
#if defined(KINC_WINDOWS)
	if (counter == 0) {
		WSACleanup();
	}
#endif
}

bool kinc_socket_select(kinc_socket_t *sock, uint32_t waittime, bool read, bool write) {
#if (defined(KINC_WINDOWS) || defined(KINC_POSIX))
	fd_set r_fds, w_fds;
	struct timeval timeout;

	FD_ZERO(&r_fds);
	FD_ZERO(&w_fds);

	FD_SET(sock->handle, &r_fds);
	FD_SET(sock->handle, &w_fds);

	timeout.tv_sec = waittime;
	timeout.tv_usec = 0;

	if (select(0, &r_fds, &w_fds, NULL, &timeout) < 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "kinc_socket_select didn't work: %s", strerror(errno));
		return false;
	}

	if (read && write) {
		return FD_ISSET(sock->handle, &w_fds) && FD_ISSET(sock->handle, &r_fds);
	}
	else if (read) {
		return FD_ISSET(sock->handle, &r_fds);
	}
	else if (write) {
		return FD_ISSET(sock->handle, &w_fds);
	}
	else {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Calling kinc_socket_select with both read and write set to false is useless.");
		return false;
	}
#else
	return false;
#endif
}

bool kinc_socket_set(kinc_socket_t *sock, const char *host, int port, kinc_socket_family_t family, kinc_socket_protocol_t protocol) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)

	sock->family = family;
	sock->protocol = protocol;
	sock->port = htons((unsigned short)port);

	if (host == NULL)
		return true;

	if (isdigit(host[0]) || (family == KINC_SOCKET_FAMILY_IP6 && host[4] == ':')) { // Is IPv4 or IPv6 string
		struct in_addr addr;

		if (inet_pton(sock->family == KINC_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6, host, &addr) == 0) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Invalid %s address: %s\n", sock->family == KINC_SOCKET_FAMILY_IP4 ? "IPv4" : "IPv6", host);
			return false;
		}
		sock->host = addr.s_addr;
		return true;
	}
	else {
		struct addrinfo *address = NULL;
		int res = resolveAddress(host, port, &address);
		if (res != 0) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
			return false;
		}
#if defined(KINC_POSIX)
		sock->host = ((struct sockaddr_in *)address->ai_addr)->sin_addr.s_addr;
#else
		sock->host = ((struct sockaddr_in *)address->ai_addr)->sin_addr.S_un.S_addr;
#endif
		freeaddrinfo(address);

		return true;
	}
#else
	return false;
#endif
}

bool kinc_socket_listen(kinc_socket_t *socket, int backlog) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	int res = listen(socket->handle, backlog);
	return (res == 0);
#else
	return false;
#endif
}

bool kinc_socket_accept(kinc_socket_t *sock, kinc_socket_t *newSocket, unsigned *remoteAddress, unsigned *remotePort) {
#if defined(KINC_WINDOWS)
	typedef int socklen_t;
#endif
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	struct sockaddr_in addr;
	socklen_t addrLength = sizeof(addr);
	newSocket->handle = accept(sock->handle, (struct sockaddr *)&addr, &addrLength);
	if (newSocket->handle <= 0) {
		return false;
	}

	newSocket->connected = sock->connected = true;
	newSocket->host = addr.sin_addr.s_addr;
	newSocket->port = addr.sin_port;
	newSocket->family = sock->family;
	newSocket->protocol = sock->protocol;
	*remoteAddress = ntohl(addr.sin_addr.s_addr);
	*remotePort = ntohs(addr.sin_port);
	return true;
#else
	return false;
#endif
}

bool kinc_socket_connect(kinc_socket_t *sock) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	struct sockaddr_in addr;
	addr.sin_family = sock->family == KINC_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6;
	addr.sin_addr.s_addr = sock->host;
	addr.sin_port = sock->port;

	int res = connect(sock->handle, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	return sock->connected = (res == 0);
#else
	return false;
#endif
}

int kinc_socket_send(kinc_socket_t *sock, const uint8_t *data, int size) {
#if defined(KINC_WINDOWS)
	typedef int socklen_t;
#endif
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	if (sock->protocol == KINC_SOCKET_PROTOCOL_UDP) {
		struct sockaddr_in addr;
		addr.sin_family = sock->family == KINC_SOCKET_FAMILY_IP4 ? AF_INET : AF_INET6;
		addr.sin_addr.s_addr = sock->host;
		addr.sin_port = sock->port;

		size_t sent = sendto(sock->handle, (const char *)data, size, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if (sent != size) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
			return -1;
		}
		return (int)sent;
	}
	else {
		if (!sock->connected) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Call kinc_sockect_connect/bind before send/recv can be called for TCP sockets.");
			return -1;
		}

		size_t sent = send(sock->handle, (const char *)data, size, 0);
		if (sent != size) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
		}
		return (int)sent;
	}
#else
	return 0;
#endif
}

int kinc_socket_send_address(kinc_socket_t *sock, unsigned address, int port, const uint8_t *data, int size) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	size_t sent = sendto(sock->handle, (const char *)data, size, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
	return (int)sent;
#else
	return 0;
#endif
}

int kinc_socket_send_url(kinc_socket_t *sock, const char *url, int port, const uint8_t *data, int size) {
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)
	struct addrinfo *address = NULL;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
		return 0;
	}

	size_t sent = sendto(sock->handle, (const char *)data, size, 0, address->ai_addr, sizeof(struct sockaddr_in));
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
	freeaddrinfo(address);
	return (int)sent;
#else
	return 0;
#endif
}

int kinc_socket_receive(kinc_socket_t *sock, uint8_t *data, int maxSize, unsigned *fromAddress, unsigned *fromPort) {
#if defined(KINC_WINDOWS)
	typedef int socklen_t;
	typedef int ssize_t;
#endif
#if defined(KINC_WINDOWS) || defined(KINC_POSIX)

	if (sock->protocol == KINC_SOCKET_PROTOCOL_UDP) {
		struct sockaddr_in from;
		socklen_t fromLength = sizeof(from);
		ssize_t bytes = recvfrom(sock->handle, (char *)data, maxSize, 0, (struct sockaddr *)&from, &fromLength);
		if (bytes <= 0) {
			return (int)bytes;
		}
		*fromAddress = ntohl(from.sin_addr.s_addr);
		*fromPort = ntohs(from.sin_port);
		return (int)bytes;
	}
	else {

		if (!sock->connected) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Call kinc_sockect_connect/bind before send/recv can be called for TCP sockets.");
			return -1;
		}
		ssize_t bytes = recv(sock->handle, (char *)data, maxSize, 0);
		*fromAddress = ntohl(sock->host);
		*fromPort = ntohs(sock->port);
		return (int)bytes;
	}
#else
	return 0;
#endif
}

unsigned kinc_url_to_int(const char *url, int port) {
#if defined(KINC_WINDOWS)
	struct addrinfo *address = NULL;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
		return -1;
	}

	unsigned fromAddress = ntohl(((struct sockaddr_in *)address->ai_addr)->sin_addr.S_un.S_addr);
	freeaddrinfo(address);

	return fromAddress;
#else
	return 0;
#endif
}

#endif
