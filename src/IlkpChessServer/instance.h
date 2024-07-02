#pragma once
#include "networking.h"

#include <chess.h>
#include <chrono>
#include <thread>
#include <array>
#include <memory>

class Instance
{
public:
	Instance(const std::vector<SOCKET>& playerSocks);
	Instance(Instance&& other) noexcept;
	Instance& operator=(Instance&& other) noexcept;

	Instance(const Instance& other) = delete;
	Instance& operator=(const Instance& other) = delete;

	inline void requestClose() { _closeRequested = true; }
	inline void waitForClose() const
	{
		while (!_isClosed)
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	inline bool isClosed() const { return _isClosed; }

private:
	enum class ServerState
	{
		a
	};

	std::thread _myThread;
	std::atomic<bool> _closeRequested = false;
	std::atomic<bool> _isClosed = false;
	std::vector<SOCKET> _observerSockets;

	void start(std::vector<SOCKET> playerSockets);

	static std::vector<char> createGameStateMsg(const GameState& gameState);
	static std::vector<char> createInitializeMsg(Side side, const GameFormat& gameFormat);
};