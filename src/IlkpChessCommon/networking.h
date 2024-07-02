#pragma once

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <string>

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

enum class ServerMsg : char
{
	ping,
	pingResponse,
	initialize,
	gameState
};
typedef std::underlying_type<ServerMsg>::type uTypeServerMsg;

enum class ClientMsg : char
{
	ping,
	pingResponse,
	move
};
typedef std::underlying_type<ClientMsg>::type uTypeClientMsg;

enum class InitializeMsgIndices : char
{
	side = 0,
	gameFormat = sizeof(char)
};
typedef std::underlying_type<InitializeMsgIndices>::type uTypeInitializeMsgIndices;