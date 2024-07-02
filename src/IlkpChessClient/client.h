#pragma once
#include <networking.h>

#include <thread>
#include <atomic>

class GameState;

class Client
{
public:
	Client(const std::string& ip, const std::string& port);
	Client(Client&& other) noexcept;
	Client& operator=(Client&& other) noexcept;
	~Client();

	Client(const Client& other) = delete;
	Client& operator=(const Client& other) = delete;

	inline void requestClose()
	{
		_closeRequested = true;
	}
	inline void waitForClose() const
	{
		while (!_isClosed)
			std::this_thread::sleep_for(std::chrono::milliseconds(_tickRate_ms));
	}

private:
	static const std::chrono::milliseconds _tickRate_ms;
	std::thread _myThread;
	std::atomic<bool> _closeRequested = false;
	std::atomic<bool> _isClosed = false;

	void start(const std::string& ip, const std::string& port);
	void writeLog(const std::string& msg) const;
	void printGameState(const GameState& gameState) const;
};