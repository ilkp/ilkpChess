#include "client.h"

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

	Client client("127.0.0.1", "5555");
	client.waitForClose();
	return 0;
}