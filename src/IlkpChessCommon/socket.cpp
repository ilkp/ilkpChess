#include "socket.h"
#include <cassert>

Socket::Socket(const std::string& name, SOCKET socket) noexcept :
    Logger(name),
    _socket(socket),
    _status(Status::disconnected),
    _nextMsgReceivedCbId(0),
    _nextStatusChangedCbId(0),
    _isReading(false)
{
}

Socket::Socket(Socket&& other) noexcept :
    Logger(std::move(other)),
    _status(other._status),
    _socket(other._socket),
    _msgReceivedCallbacks(std::move(other._msgReceivedCallbacks)),
    _statusChangedCallbacks(std::move(other._statusChangedCallbacks))
{
    stopReading();
    other.stopReading();
    _nextMsgReceivedCbId.store(other._nextMsgReceivedCbId.load());
    _nextStatusChangedCbId.store(other._nextStatusChangedCbId.load());
    _isReading.store(other._isReading.load());

    other._socket = invalidSocket();
    other._status = Status::disconnected;
    other._nextMsgReceivedCbId = 0;
    other._nextStatusChangedCbId = 0;
    other._isReading = false;

    if (_isReading)
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
        _nextMsgReceivedCbId = other._nextMsgReceivedCbId.load();
        _nextStatusChangedCbId = other._nextStatusChangedCbId.load();

        other._socket = invalidSocket();
        other._status = Status::disconnected;
        other._nextMsgReceivedCbId = 0;
        other._nextStatusChangedCbId = 0;
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

void Socket::startReading()
{
    _isReading = true;
    _readThread = std::thread(&Socket::readLoop, this);
}

void Socket::stopReading()
{
    _isReading = false;
    if (_readThread.joinable())
        _readThread.join();
}

void Socket::write(char id, const std::string& msg)
{
    std::unique_lock lock(_writeMutex);
    std::string sizeMsg = std::to_string(msg.size());
    sizeMsg.insert(0, msgSizeBytes - sizeMsg.size(), '0');
    writeBytes(sizeMsg.data(), sizeMsg.size());
    writeBytes(&id, sizeof(char));
    writeBytes(msg.data(), msg.size());
}

void Socket::write(char id, const char* msg, size_t bytes)
{
    std::unique_lock lock(_writeMutex);
    std::string sizeMsg = std::to_string(bytes);
    sizeMsg.insert(0, msgSizeBytes - sizeMsg.size(), '0');
    writeBytes(sizeMsg.data(), sizeMsg.size());
    writeBytes(&id, sizeof(char));
    writeBytes(msg, bytes);
}

uint64_t Socket::subStatusChangedCallback(StatusChangedCallback callback)
{
    callback(_status);
    _statusChangedCallbacks.insert({ _nextStatusChangedCbId++, std::move(callback) });
    return _nextStatusChangedCbId;
}

void Socket::unsubStatusChangedCallback(uint64_t callbackId)
{
    _statusChangedCallbacks.erase(callbackId);
}

void Socket::notify(Status status)
{
    std::unique_lock lock(_statusMutex);
    _status = status;
    for (auto& [id, callback] : _statusChangedCallbacks)
        callback(status);
}

void Socket::notify(char id, const std::string& msg) const
{
    for (auto& [callbackId, callback] : _msgReceivedCallbacks)
        callback(msg);
}

void Socket::readLoop()
{
    const struct timeval timeout { .tv_sec = 0, .tv_usec = 100000 };
    while (_isReading)
    {
        fd_set readSet;
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
            const ReadResult msgSizeRead = readBytes(_socket, msgSizeBytes);
            if (msgSizeRead.resultCode != ResultCode::success)
            {
                writeLog("reading msg size returned error: " + std::to_string(GETSOCKETERRNO()));
                _isReading = false;
                break;
            }

            const ReadResult idRead = readBytes(_socket, sizeof(char));
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
            notify(idRead.msg.at(0), msgRead.msg);
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