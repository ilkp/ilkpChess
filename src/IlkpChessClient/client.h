#pragma once
#include "logger.h"

#include <thread>

class Client : public Logger
{
public:
	Client(const std::string& name, const std::string& ip, const std::string& port);
	~Client();

	void requestClose();
	void waitForClose();

private:
	void mainLoop(const std::string& ip, const std::string& port);

	std::thread _mainThread;
	std::atomic_bool _closeRequested;
};