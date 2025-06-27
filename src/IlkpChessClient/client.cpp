#include "client.h"
#include "sock_defs.h"
#include "socket.h"

Client::Client(const std::string& name, const std::string& ip, const std::string& port) :
	Logger(name),
	_closeRequested(false),
	_mainThread(std::thread(&Client::mainLoop, this, ip, port))
{
}

Client::~Client()
{
	_closeRequested = true;
	waitForClose();
}

void Client::requestClose()
{
	writeLog("close requested");
	_closeRequested = true;
}

void Client::waitForClose()
{
	if (_mainThread.joinable())
		_mainThread.join();
}

void Client::mainLoop(const std::string& ip, const std::string& port)
{
	writeLog("starting");
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* peerAddress;
	getaddrinfo(ip.c_str(), port.c_str(), &hints, &peerAddress);

	char addressBuffer[100];
	char serviceBuffer[100];
	getnameinfo(
		peerAddress->ai_addr,
		peerAddress->ai_addrlen,
		addressBuffer, sizeof(addressBuffer),
		serviceBuffer, sizeof(serviceBuffer),
		NI_NUMERICHOST);

	SOCKET serverSocket = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
	if (!ISVALIDSOCKET(serverSocket))
	{
		writeLog("creating socket failed");
		return;
	}

	int result;
	do
	{
		result = connect(serverSocket, peerAddress->ai_addr, peerAddress->ai_addrlen);
		if (result != 0)
		{
			const int error = WSAGetLastError();
			LPVOID msgBuffer;
			const DWORD size = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&msgBuffer,
				0,
				nullptr
			);

			writeLog("connect failed: " + std::to_string(error) + " " + std::string((LPSTR)msgBuffer, size));
			LocalFree(msgBuffer);
		}
	} while (!_closeRequested && result != 0);
	freeaddrinfo(peerAddress);

	Socket serverSock("ServerSock", serverSocket);
	serverSock.startReading();

	while (!_closeRequested)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	writeLog("closing");
}
