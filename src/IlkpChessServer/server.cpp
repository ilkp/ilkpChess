#include "server.h"

#include "instance.h"

#include <chess.h>
#include <networking.h>

#include <future>
#include <chrono>
#include <iostream>
#include <format>
#include <queue>

Server::Server(const std::string& listenPort)
{
	_myThread = std::thread(&Server::start, this, listenPort);
}

Server::~Server()
{
}

void Server::start(const std::string& port)
{
	writeLog("Starting server");
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
	writeLog("Listening for connections");
	
	std::future<SOCKET> pendingSocket = std::async(accept, listenSocket, nullptr, nullptr);
	std::queue<SOCKET> waitingSockets;
	std::vector<Instance> ongoingGames;
	while (!_closeRequested)
	{
		if (pendingSocket.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready)
		{
			writeLog("User connected");
			waitingSockets.push(pendingSocket.get());
			pendingSocket = std::async(accept, listenSocket, nullptr, nullptr);
		}
		if (waitingSockets.size() >= static_cast<uTypeSide>(Side::nSides))
		{
			std::vector<SOCKET> toPlaySockets;
			//while (toPlaySockets.size() < 2 || waitingSockets.size() >= 2)
			//{
			//	char ping = static_cast<char>(ServerMsg::ping);
			//	SOCKET toPlaySocket = waitingSockets.front();
			//	send(toPlaySocket, &ping, sizeof(char), );
			//}
			writeLog("Starting new game");
			SOCKET s1 = waitingSockets.front();
			waitingSockets.pop();
			SOCKET s2 = waitingSockets.front();
			waitingSockets.pop();
			ongoingGames.push_back(Instance({ s1, s2 }));
		}
		for (auto it = ongoingGames.begin(); it != ongoingGames.end(); ++it)
			if (it->isClosed())
				it = ongoingGames.erase(it);
	}

	writeLog("Closing");
	shutdown(listenSocket, SD_RECEIVE);
	CLOSESOCKET(listenSocket);

	for (Instance& instance : ongoingGames)
		instance.requestClose();

	for (const Instance& instance : ongoingGames)
		instance.waitForClose();

	_isClosed = true;
}

void Server::writeLog(const std::string& msg) const
{
	std::cout << std::format("{:%F %T} ", std::chrono::system_clock::now()) << msg << std::endl;
}



//void Server::nextState()
//{
//	bool closing = false;
//	Blackboard blackboard;
//	blackboard.gameState = defaultStartGameState();
//	while (!closing)
//	{
//		switch (_serverState)
//		{
//			case State::closing:
//			{
//				closing = true;
//				_closing = true;
//				break;
//			}
//			case State::connectPlayers:
//			{
//				connectPlayers(blackboard);
//				break;
//			}
//			case State::gameOver:
//			{
//				gameOver();
//				break;
//			}
//		}
//	}
//}
//
//void Server::connectPlayers(Blackboard& blackboard)
//{
//	std::cout << "entered 'starting'" << std::endl;
//	addrinfo hints;
//	memset(&hints, 0, sizeof(hints));
//	hints.ai_family = AF_INET;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_flags = AI_PASSIVE;
//
//	addrinfo* bindAddress;
//	getaddrinfo(0, _listenPort.c_str(), &hints, &bindAddress);
//
//	_listenSocket = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
//	bind(_listenSocket, bindAddress->ai_addr, bindAddress->ai_addrlen);
//	freeaddrinfo(bindAddress);
//
//	listen(_listenSocket, 10);
//	std::array<std::future<SOCKET>, 2> pendingPlayerSockets;
//	for (auto& socket : pendingPlayerSockets)
//		socket = std::async(accept, _listenSocket, nullptr, nullptr);
//
//	while (_serverState == Server::State::connectPlayers)
//	{
//		bool socketsReady = true;
//		for (const auto& socket : pendingPlayerSockets)
//			socketsReady = socketsReady && socket.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
//
//		if (_closeRequested)
//		{
//			shutdown(_listenSocket, SD_RECEIVE);
//			_serverState = Server::State::closing;
//		}
//		else if (socketsReady)
//		{
//			for (size_t i = 0; i < pendingPlayerSockets.size(); ++i)
//				blackboard.playerSockets[i] = pendingPlayerSockets.at(i).get();
//			_serverState = Server::State::playing;
//			blackboard.startTime = std::chrono::steady_clock::now();
//		}
//	}
//}
//
//void Server::gameOver()
//{
//	std::cout << "entered 'gameOver'" << std::endl;
//}
//
//void Server::closing()
//{
//	std::cout << "entered 'closing'" << std::endl;
//}
