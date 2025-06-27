#pragma once
#include "logger.h"

#include <thread>

class Client : public Logger
{
public:
	Client(const std::string& name, const std::string& ip, const std::string& port);
	~Client();
	void waitForClose() const;
	void requestClose();

private:
	void mainLoop(const std::string& ip, const std::string& port);

	std::thread _mainThread;
	std::atomic_bool _closeRequested;
	std::atomic_bool _isClosed;
};