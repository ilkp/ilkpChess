#include "listen_socket.h"

ListenSocket::ListenSocket(const std::string& name, const std::string& port) :
	Logger(name),
	_closeRequested(false)
{
	_myThread = std::thread(&ListenSocket::listenConnections, this, port);
}

ListenSocket::~ListenSocket()
{
	_closeRequested = true;
	waitForClose();
}

void ListenSocket::requestClose()
{
	_closeRequested = true;
}

Socket ListenSocket::nextSocket()
{
	std::unique_lock lock(_acceptedSocketsMutex);
	if (_acceptedSockets.empty())
		return Socket("InvalidSocket", invalidSocket());
	Socket socket = std::move(_acceptedSockets.back());
	_acceptedSockets.pop_back();
	return socket;
}

void ListenSocket::listenConnections(const std::string& port)
{
	writeLog("starting");
	addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	addrinfo* bindAddress;
	if (getaddrinfo(0, port.c_str(), &hints, &bindAddress) != 0)
	{
		writeLog("getaddrinfo() failed with code " + std::to_string(GETSOCKETERRNO()));
		return;
	}
	SOCKET listenSocket = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
	if (!ISVALIDSOCKET(listenSocket))
	{
		writeLog("socket() failed with code " + std::to_string(GETSOCKETERRNO()));
		freeaddrinfo(bindAddress);
		return;
	}
	if (bind(listenSocket, bindAddress->ai_addr, bindAddress->ai_addrlen) < 0)
	{
		writeLog("bind() failed with code " + std::to_string(GETSOCKETERRNO()));
		freeaddrinfo(bindAddress);
		CLOSESOCKET(listenSocket);
		return;
	}
	freeaddrinfo(bindAddress);
	if (listen(listenSocket, 10) < 0)
	{
		writeLog("listen() failed with code " + std::to_string(GETSOCKETERRNO()));
		CLOSESOCKET(listenSocket);
		return;
	}
	writeLog("ready for connections");

	timeval selectTimeout{ .tv_sec = 0, .tv_usec = 500000 };
	while (!_closeRequested)
	{
		fd_set reads{};
		FD_ZERO(&reads);
		FD_SET(listenSocket, &reads);

		const int selectResult = select(listenSocket + 1, &reads, nullptr, nullptr, &selectTimeout);
		if (selectResult > 0)
		{
			if (FD_ISSET(listenSocket, &reads))
			{
				sockaddr_in clientAddress{};
				socklen_t clientAddressLen = sizeof(clientAddress);
				const SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddress, &clientAddressLen);
				if (ISVALIDSOCKET(clientSocket))
				{
					std::unique_lock lock(_acceptedSocketsMutex);
					_acceptedSockets.push_front(Socket("NewSocket", clientSocket));
					char clientIp[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, sizeof(clientIp));
					writeLog("new connection from " + std::string(clientIp) + ":" + std::to_string(ntohs(clientAddress.sin_port)));
				}
			}
		}
		else if (selectResult == 0)
		{
			// normal timeout
			continue;
		}
		else
		{
			writeLog("select() failed with code " + std::to_string(GETSOCKETERRNO()));
			break;
		}
	}

	if (shutdown(listenSocket, SD_BOTH) == SOCKETERROR)
		writeLog("shutdown() listen socket failed with code " + std::to_string(GETSOCKETERRNO()));

	CLOSESOCKET(listenSocket);
}

void ListenSocket::waitForClose()
{
	if (_myThread.joinable())
		_myThread.join();
}
