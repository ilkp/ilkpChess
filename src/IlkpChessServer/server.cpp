#include "server.h"
#include "listen_socket.h"
#include "chess.h"
#include "player.h"

Server::Server(const std::string& name, const std::string& port) : 
	Logger(name),
	_closeRequested(false),
	_tickRate(1000/30),
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
	GameState gameState = GameState::defaultStartingState();
	ListenSocket listenSocket("ListenSocket", port);
	std::vector<Player> players;
	while (players.size() < 2)
	{
		Socket socket = listenSocket.nextSocket();
		if (socket.isValid())
		{
			const std::string ip = socket.getIp();
			Player newPlayer{ .socket = std::move(socket), .ip = ip, .playingAs = (Side)players.size() };
			newPlayer.socket.write(SocketMsgId::gameState, reinterpret_cast<const char*>(&gameState), sizeof(GameState));
			newPlayer.socket.write(SocketMsgId::youPlayAs, reinterpret_cast<const char*>(&newPlayer.playingAs), sizeof(Side));
			players.push_back(std::move(newPlayer));
		}
		else
			std::this_thread::sleep_for(_tickRate);
	}

	for (Player& player : players)
		player.socket.write(SocketMsgId::startGame, "");

	while (!_closeRequested)
	{
		players[0].socket.write(SocketMsgId::ping, "");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	writeLog("closing");
}
