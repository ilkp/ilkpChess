#include "sock_defs.h"
#include "server.h"

#include <iostream>

int main(int argc, char* argv[])
{
#if defined(_WIN32)
	WSAData wsaData;
	if (int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData); wsaResult != 0)
	{
		std::cout << "Failed to initialize Winsock: " << wsaResult << ", " << WSAGetLastError() << std::endl;
		return -1;
	}
#endif
	const std::string port = argc > 1 ? argv[1] : "5555";
	Server server("Server", port);
	server.waitForClose();
#if defined(_WIN32)
	WSACleanup();
#endif
	return 0;
}