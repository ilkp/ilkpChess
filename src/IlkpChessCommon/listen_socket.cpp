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

std::optional<Socket> ListenSocket::nextSocket()
{
	std::unique_lock lock(_acceptedSocketsMutex);
	if (_acceptedSockets.empty())
		return {};
	SOCKET socket = _acceptedSockets.back();
	_acceptedSockets.pop_back();
	return Socket("Socket", socket);
}

void ListenSocket::listenConnections(const std::string& port)
{
	writeLog("starting");
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	addrinfo* bindAddress;
	getaddrinfo(0, port.c_str(), &hints, &bindAddress);
	SOCKET listenSocket = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
	bind(listenSocket, bindAddress->ai_addr, bindAddress->ai_addrlen);
	freeaddrinfo(bindAddress);
	listen(listenSocket, 10);

	fd_set master;
	FD_ZERO(&master);
	FD_SET(listenSocket, &master);
	SOCKET maxSocket = listenSocket;
	writeLog("waiting for connections");

	while (!_closeRequested)
	{
		fd_set reads;
		reads = master;
		timeval timeout{ .tv_sec = 0, .tv_usec = 500000 };
		select(maxSocket + 1, &reads, nullptr, nullptr, &timeout);

		if (FD_ISSET(listenSocket, &reads))
		{
			const SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
			if (ISVALIDSOCKET(clientSocket))
			{
				std::unique_lock lock(_acceptedSocketsMutex);
				_acceptedSockets.push_front(clientSocket);
				writeLog("new connection accepted");
			}
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	std::unique_lock lock(_acceptedSocketsMutex);
	for (SOCKET socket : _acceptedSockets)
		CLOSESOCKET(socket);

	shutdown(listenSocket, SD_RECEIVE);
	CLOSESOCKET(listenSocket);
}

void ListenSocket::waitForClose()
{
	if (_myThread.joinable())
		_myThread.join();
}
