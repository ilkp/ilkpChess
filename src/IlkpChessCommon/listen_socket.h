#pragma once
#include "sock_defs.h"
#include "socket.h"
#include "logger.h"

#include <string>
#include <thread>
#include <future>
#include <deque>
#include <mutex>
#include <optional>

class ListenSocket : public Logger
{
public:
	ListenSocket(const std::string& name, const std::string& port);
	~ListenSocket();

	void requestClose();
	Socket nextSocket();

private:
	void listenConnections(const std::string& port);
	void waitForClose();

	std::thread _myThread;
	std::deque<Socket> _acceptedSockets;
	std::mutex _acceptedSocketsMutex;
	bool _closeRequested;
};
