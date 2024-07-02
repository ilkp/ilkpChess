#include "instance.h"

Instance::Instance(const std::vector<SOCKET>& playerSockets)
{
	_myThread = std::thread(&Instance::start, this, playerSockets);
}

Instance::Instance(Instance&& other) noexcept
{
	_myThread.swap(other._myThread);
	_observerSockets = std::exchange(other._observerSockets, std::vector<SOCKET>());
	_closeRequested = other._closeRequested.load();
	other._closeRequested = false;
	_isClosed = other._isClosed.load();
	other._isClosed = false;
}

Instance& Instance::operator=(Instance&& other) noexcept
{
	if (this != &other)
	{
		_myThread.swap(other._myThread);
		_observerSockets = std::exchange(other._observerSockets, std::vector<SOCKET>());
		_closeRequested = other._closeRequested.load();
		other._closeRequested = false;
		_isClosed = other._isClosed.load();
		other._isClosed = false;
	}

	return *this;
}

void Instance::start(std::vector<SOCKET> playerSockets)
{
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::vector<int> clocks_ms(playerSockets.size());
	GameState gameState = GameState::defaultStartingState();
	GameFormat gameFormat;
	for (size_t i = 0; i < playerSockets.size(); ++i)
	{
		std::vector<char> initializeMsg = createInitializeMsg(Side(i), gameFormat);
		send(playerSockets.at(i), initializeMsg.data(), initializeMsg.size(), 0);
	}

	std::vector<char> gameStateMsg = createGameStateMsg(gameState);
	for (size_t i = 0; i < playerSockets.size(); ++i)
		send(playerSockets.at(i), gameStateMsg.data(), gameStateMsg.size(), 0);
}

std::vector<char> Instance::createGameStateMsg(const GameState& gameState)
{
	std::vector<char> msg;
	msg.resize(sizeof(GameState) + 1);
	msg[0] = static_cast<char>(ServerMsg::gameState);
	std::memcpy(&msg[1], &gameState, sizeof(GameState));
	return msg;
}

std::vector<char> Instance::createInitializeMsg(Side side, const GameFormat& gameFormat)
{
	std::vector<char> msg;
	msg.resize(1 + sizeof(Side) + sizeof(GameFormat));
	msg[0] = static_cast<char>(ServerMsg::initialize);
	std::memcpy(&msg[1], &side, sizeof(Side));
	std::memcpy(&msg[1 + sizeof(Side)], &gameFormat, sizeof(GameFormat));
	return msg;
}
