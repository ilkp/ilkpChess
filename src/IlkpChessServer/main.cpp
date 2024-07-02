#pragma once
#include "server.h"

#include <networking.h>

#include <iostream>

int main(int argc, char** argv)
{
#if defined(_WIN32)
	int wsaResult;
	WSAData wsaData;
	wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaResult != 0)
	{
		std::cout << "Failed to initialize Winsock: " << wsaResult << ", " << WSAGetLastError() << std::endl;
		return -1;
	}
#endif

	std::string port;
	if (argc > 1)
		port = argv[1];
	else
		port = "5555";

	Server server(port);
	server.waitForClose();

	return 0;
}