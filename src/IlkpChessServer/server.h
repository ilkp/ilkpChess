#pragma once
#include "logger.h"

#include <atomic>
#include <thread>

class Server : public Logger
{
public:
	explicit Server(const std::string& name, const std::string& port);
	~Server();
	void waitForClose() const;
	void mainLoop(const std::string& port);

private:
	std::thread _mainThread;
	std::atomic_bool _isClosed;
	std::atomic_bool _closeRequested;
};