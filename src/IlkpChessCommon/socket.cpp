#include "socket.h"
#include <algorithm>

Socket::Socket(const std::string& name, SOCKET socket) noexcept :
    Logger(name),
    _socket(socket),
    _status(Status::disconnected),
    _isReading(false),
    _nextMsgReceivedCbToken(0),
    _nextStatusChangedCbToken(0)
{
}

Socket::Socket(Socket&& other) noexcept :
    Logger(std::move(other)),
    _status(other._status),
    _socket(other._socket),
    _msgReceivedCallbacks(std::move(other._msgReceivedCallbacks)),
    _statusChangedCallbacks(std::move(other._statusChangedCallbacks)),
    _nextMsgReceivedCbToken(other._nextMsgReceivedCbToken.load()),
    _nextStatusChangedCbToken(other._nextStatusChangedCbToken.load())
{
    const bool otherWasReading = other._isReading.load();
    stopReading();
    other.stopReading();
    other._socket = invalidSocket();
    other._status = Status::disconnected;
    other._nextMsgReceivedCbToken = 0;
    other._nextStatusChangedCbToken = 0;

    if (otherWasReading)
        startReading();
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        std::scoped_lock lock(_writeMutex, _statusMutex);
        std::scoped_lock lockOther(other._writeMutex, other._statusMutex);
        Logger::operator=(std::move(other));
        _isReading = other._isReading.load();
        stopReading();
        other.stopReading();
        _socket = other._socket;
        _status = other._status;
        _msgReceivedCallbacks = std::move(other._msgReceivedCallbacks);
        _statusChangedCallbacks = std::move(other._statusChangedCallbacks);
        _nextMsgReceivedCbToken = other._nextMsgReceivedCbToken.load();
        _nextStatusChangedCbToken = other._nextStatusChangedCbToken.load();

        other._socket = invalidSocket();
        other._status = Status::disconnected;
        other._nextMsgReceivedCbToken = 0;
        other._nextStatusChangedCbToken = 0;
        other._isReading = false;

        if (_isReading)
            startReading();
    }
    return *this;
}

Socket::~Socket()
{
    if (ISVALIDSOCKET(_socket))
    {
        CLOSESOCKET(_socket);
        _socket = invalidSocket();
    }
}

bool Socket::isValid() const
{
    return ISVALIDSOCKET(_socket);
}

std::string Socket::getIp() const
{
    std::string ip = "";
    if (isValid())
    {
        sockaddr_in address{};
        socklen_t addressLen = sizeof(sockaddr);
        if (getpeername(_socket, (sockaddr*)(&address), &addressLen) == 0)
        {
            ip.resize(INET_ADDRSTRLEN, '0');
            inet_ntop(AF_INET, &address.sin_addr, ip.data(), ip.size());
        }
    }
    return ip;
}

void Socket::startReading()
{
    _isReading = true;
    if (!_readThread.joinable())
        _readThread = std::thread(&Socket::readLoop, this);
}

void Socket::stopReading()
{
    _isReading = false;
    if (_readThread.joinable())
        _readThread.join();
}

void Socket::write(SocketMsgId id, const std::string& msg)
{
    std::unique_lock lock(_writeMutex);
    std::string sizeMsg = std::to_string(msg.size());
    sizeMsg.insert(0, sizeMsgBytes - sizeMsg.size(), '0');
    writeBytes(sizeMsg.data(), sizeMsg.size());
    writeBytes(reinterpret_cast<const char*>(&id), sizeof(SocketMsgIdType));
    writeBytes(msg.data(), msg.size());
}

void Socket::write(SocketMsgId id, const char* msg, size_t bytes)
{
    std::unique_lock lock(_writeMutex);
    std::string sizeMsg = std::to_string(bytes);
    sizeMsg.insert(0, sizeMsgBytes - sizeMsg.size(), '0');
    writeBytes(sizeMsg.data(), sizeMsg.size());
    writeBytes(reinterpret_cast<const char*>(&id), sizeof(SocketMsgIdType));
    writeBytes(msg, bytes);
}

CallbackToken Socket::subStatusChangedCallback(StatusChangedCallback callback)
{
    callback(_status);
    _statusChangedCallbacks.insert({ _nextStatusChangedCbToken, std::move(callback) });
    return _nextStatusChangedCbToken++;
}

void Socket::unsubStatusChangedCallback(CallbackToken token)
{
    _statusChangedCallbacks.erase(token);
}

CallbackToken Socket::subMsgReceivedCallback(SocketMsgId id, MsgReceivedCallback callback)
{
    if (_msgReceivedCbTokens.count(id) == 0)
        _msgReceivedCbTokens.insert({ id, std::vector<CallbackToken>{ _nextMsgReceivedCbToken } });

    _msgReceivedCallbacks.insert({ _nextMsgReceivedCbToken, std::move(callback) });
    return _nextMsgReceivedCbToken++;
}

void Socket::unsubMsgReceivedCallback(SocketMsgId id, CallbackToken token)
{
    if (auto msgIt = _msgReceivedCbTokens.find(id); msgIt != _msgReceivedCbTokens.end())
    {
        if (auto tokenIt = std::find(msgIt->second.begin(), msgIt->second.end(), token); tokenIt != msgIt->second.end())
        {
            _msgReceivedCallbacks.erase(*tokenIt);
            msgIt->second.erase(tokenIt);
        }
    }
}

void Socket::notify(Status status)
{
    std::unique_lock lock(_statusMutex);
    _status = status;
    for (auto& [callbackToken, callback] : _statusChangedCallbacks)
        callback(status);
}

void Socket::notify(SocketMsgId id, const std::string& msg) const
{
    if (auto it = _msgReceivedCbTokens.find(id); it != _msgReceivedCbTokens.end())
        for (const CallbackToken& token : it->second)
            if (auto cbIt = _msgReceivedCallbacks.find(token); cbIt != _msgReceivedCallbacks.end())
                cbIt->second(msg);
}

void Socket::readLoop()
{
    const struct timeval timeout { .tv_sec = 0, .tv_usec = 100000 };
    while (_isReading)
    {
        fd_set readSet{};
        FD_ZERO(&readSet);
        FD_SET(_socket, &readSet);
        
        if (const int result = select(_socket + 1, &readSet, 0, 0, &timeout); result <= 0)
        {
            if (result == 0)
            {
                // timeout
                continue;
            }
            else if (result < 0)
            {
                writeLog("select() returned error: " + std::to_string(GETSOCKETERRNO()));
                _isReading = false;
            }
        }

        if (FD_ISSET(_socket, &readSet))
        {
            const ReadResult msgSizeRead = readBytes(_socket, sizeMsgBytes);
            if (msgSizeRead.resultCode != ResultCode::success)
            {
                writeLog("reading msg size returned error: " + std::to_string(GETSOCKETERRNO()));
                _isReading = false;
                break;
            }

            const ReadResult idRead = readBytes(_socket, sizeof(SocketMsgIdType));
            if (idRead.resultCode != ResultCode::success)
            {
                writeLog("reading msg id returned error: " + std::to_string(GETSOCKETERRNO()));
                _isReading = false;
                break;
            }

            const ReadResult msgRead = readBytes(_socket, std::stoll(msgSizeRead.msg));
            if (msgRead.resultCode != ResultCode::success)
            {
                writeLog("reading msg returned error: " + std::to_string(GETSOCKETERRNO()));
                _isReading = false;
                break;
            }

            writeLog("received id " + std::to_string(idRead.msg.at(0)));
            const SocketMsgId msgId = static_cast<SocketMsgId>(idRead.msg[0]);
            notify(msgId, msgRead.msg);
        }
    }
}

int Socket::writeBytes(const char* data, size_t bytes) const
{
    size_t msgBytesSent = 0;
    while (msgBytesSent < bytes)
    {
        const int sent = send(_socket, data + msgBytesSent, bytes - msgBytesSent, 0);
        if (sent == SOCKETERROR)
            return sent;
        msgBytesSent += sent;
    }
    return msgBytesSent;
}

Socket::ReadResult Socket::readBytes(SOCKET socket, size_t bytes) const
{
    std::string receiveBuffer(bytes, '\0');
    size_t totalBytesRead = 0;
    while (totalBytesRead < bytes)
    {
        const int bytesRead = recv(socket, receiveBuffer.data() + totalBytesRead, bytes - totalBytesRead, 0);
        if (bytesRead < 0)
            return ReadResult{ ResultCode::error, std::move(receiveBuffer) };
        else if (bytesRead == 0)
            return ReadResult{ ResultCode::closed, std::move(receiveBuffer) };
        totalBytesRead += bytesRead;
    }
    return ReadResult{ ResultCode::success, std::move(receiveBuffer) };
}
