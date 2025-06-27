#pragma once

#include "sock_defs.h"
#include "logger.h"

#include <cstdint>
#include <functional>
#include <string>
#include <mutex>
#include <atomic>

class Socket : Logger
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
	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;
	~Socket();

	void startReading();
	void stopReading();
	void write(const std::string& msg);
	uint64_t subStatusChangedCallback(StatusChangedCallback callback);
	void unsubStatusChangedCallback(uint64_t callbackId);

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
	void notify(const std::string& msg);
	void readLoop();
	ReadResult readBytes(SOCKET socket, size_t bytes) const;

	Status _status;
	SOCKET _socket;
	std::thread _readThread;
	std::mutex _writeMutex;
	std::mutex _statusMutex;
	std::atomic<uint64_t> _nextMsgReceivedCbId;
	std::atomic<uint64_t> _nextStatusChangedCbId;
	std::atomic_bool _isReading;
	std::unordered_map<uint64_t, MsgReceivedCallback> _msgReceivedCallbacks;
	std::unordered_map<uint64_t, StatusChangedCallback> _statusChangedCallbacks;
};