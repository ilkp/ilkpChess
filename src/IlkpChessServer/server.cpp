#include "server.h"
#include "listen_socket.h"
#include "chess.h"

Server::Server(const std::string& name, const std::string& port) : 
	Logger(name),
	_closeRequested(false),
	_mainThread(std::thread(&Server::mainLoop, this, port))
{
}

Server::~Server()
{
	_closeRequested = true;
	waitForClose();
}

void Server::requestClose()
{
	_closeRequested = true;
}

void Server::waitForClose()
{
	if (_mainThread.joinable())
		_mainThread.join();
}

void Server::mainLoop(const std::string& port)
{
	writeLog("starting");
	ListenSocket listenSocket("ListenSocket", port);
	std::vector<Socket> playerSockets;
	while (playerSockets.size() < 1)
	{
		std::optional<Socket> socket = listenSocket.nextSocket();
		if (socket.has_value())
			playerSockets.push_back(std::move(socket.value()));
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	GameState gameState = GameState::defaultStartingState();
	playerSockets[0].write(SocketMsgId::gameState, reinterpret_cast<const char*>(&gameState), sizeof(GameState));

	while (!_closeRequested)
	{
		playerSockets[0].write(SocketMsgId::ping, "");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	writeLog("closing");
}
