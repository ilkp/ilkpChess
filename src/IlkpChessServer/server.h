#pragma once

#include <string>
#include <thread>

class Server
{
public:

	Server(const std::string& listenPort = "5555");
	Server(const Server& other) = delete;
	Server(Server&& other) = delete;
	Server& operator=(const Server& other) = delete;
	Server& operator=(Server&& other) = delete;
	~Server();

	inline void requestClose() { _closeRequested = true; }
	inline void waitForClose() const
	{
		while (!_isClosed)
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

private:
	std::thread _myThread;
	std::atomic<bool> _closeRequested = false;
	std::atomic<bool> _isClosed = false;

	void start(const std::string& port);
	void writeLog(const std::string& msg) const;
};