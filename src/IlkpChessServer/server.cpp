#include "server.h"
#include "listen_socket.h"

Server::Server(const std::string& name, const std::string& port) : 
	Logger(name),
	_isClosed(false),
	_closeRequested(false),
	_mainThread(std::thread(&Server::mainLoop, this, port))
{
}

Server::~Server()
{
	_mainThread.join();
}

void Server::waitForClose() const
{
	while (!_isClosed)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

	while (!_closeRequested)
	{
		playerSockets[0].write("Testi viesti");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	writeLog("closing");
	_isClosed = true;
}
