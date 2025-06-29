#pragma once

#include "sock_defs.h"
#include "logger.h"
#include "callback_token.h"

#include <cstdint>
#include <functional>
#include <string>
#include <mutex>
#include <atomic>

using SocketMsgIdType = char;

enum SocketMsgId : SocketMsgIdType
{
	ping,
	gameState,
	youPlayAs,
	startGame
};

class Socket : public Logger
{
public:
	enum Status
	{
		disconnected,
		connected
	};

	using MsgReceivedCallback = std::function<void(const std::string&)>;
	using StatusChangedCallback = std::function<void(Status status)>;

	Socket(const std::string& name, SOCKET socket) noexcept;
	Socket(const Socket&) = delete;
	Socket(Socket&& other) noexcept;
	Socket& operator=(const Socket&) = delete;
	Socket& operator=(Socket&& other) noexcept;
	~Socket();

	bool isValid() const;
	std::string getIp() const;
	void startReading();
	void stopReading();
	void write(SocketMsgId id, const std::string& msg);
	void write(SocketMsgId id, const char* msg, size_t bytes);
	CallbackToken subStatusChangedCallback(StatusChangedCallback callback);
	void unsubStatusChangedCallback(CallbackToken token);
	CallbackToken subMsgReceivedCallback(SocketMsgId id, MsgReceivedCallback callback);
	void unsubMsgReceivedCallback(SocketMsgId id, CallbackToken token);

private:
	enum class ResultCode
	{
		error,
		closed,
		success
	};

	struct ReadResult
	{
		ResultCode resultCode;
		std::string msg;
	};

	void notify(Status status);
	void notify(SocketMsgId id, const std::string& msg) const;
	void readLoop();
	int writeBytes(const char* data, size_t bytes) const;
	ReadResult readBytes(SOCKET socket, size_t bytes) const;

	Status _status;
	SOCKET _socket;
	std::thread _readThread;
	std::mutex _writeMutex;
	std::mutex _statusMutex;
	std::atomic_bool _isReading;
	std::atomic<CallbackToken> _nextMsgReceivedCbToken;
	std::atomic<CallbackToken> _nextStatusChangedCbToken;
	std::unordered_map<SocketMsgId, std::vector<CallbackToken>> _msgReceivedCbTokens;
	std::unordered_map<CallbackToken, MsgReceivedCallback> _msgReceivedCallbacks;
	std::unordered_map<CallbackToken, StatusChangedCallback> _statusChangedCallbacks;
};