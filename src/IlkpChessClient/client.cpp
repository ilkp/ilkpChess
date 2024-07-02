#include "client.h"

#include <chess.h>

#include <iostream>
#include <format>
#include <vector>
#include <future>
#include <chrono>
#include <sstream>

const std::chrono::milliseconds Client::_tickRate_ms = std::chrono::milliseconds(1000 / 60);

Client::Client(const std::string& ip, const std::string& port)
{
	_myThread = std::thread(&Client::start, this, ip, port);
}

Client::Client(Client&& other) noexcept
{
	_myThread.swap(other._myThread);
	_closeRequested = other._closeRequested.load();
	other._closeRequested = false;
	_isClosed = other._isClosed.load();
	other._isClosed = false;
}

Client& Client::operator=(Client&& other) noexcept
{
	if (this != &other)
	{
		_myThread.swap(other._myThread);
		_closeRequested = other._closeRequested.load();
		other._closeRequested = false;
		_isClosed = other._isClosed.load();
		other._isClosed = false;
	}
	return *this;
}

Client::~Client()
{
}

void Client::start(const std::string& ip, const std::string& port)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* peerAddress;
	getaddrinfo(ip.c_str(), port.c_str(), &hints, &peerAddress);

	char addressBuffer[100];
	char serviceBuffer[100];
	getnameinfo(
		peerAddress->ai_addr,
		peerAddress->ai_addrlen,
		addressBuffer, sizeof(addressBuffer),
		serviceBuffer, sizeof(serviceBuffer),
		NI_NUMERICHOST);

	SOCKET serverSocket = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
	connect(serverSocket, peerAddress->ai_addr, peerAddress->ai_addrlen);
	freeaddrinfo(peerAddress);

	bool closing = false;
	while (!_closeRequested)
	{
		std::vector<char> recvMsg;
		recvMsg.resize(128);
		int bytes = recv(serverSocket, recvMsg.data(), 128, 0);
		ServerMsg serverMsg = static_cast<ServerMsg>(recvMsg.at(0));
		switch (serverMsg)
		{
			case ServerMsg::ping:
			{
				char ping = static_cast<char>(ClientMsg::pingResponse);
				send(serverSocket, &ping, sizeof(char), 0);
				break;
			}
			case ServerMsg::initialize:
			{
				break;
			}
			case ServerMsg::gameState:
			{
				GameState gameState(*(GameState*)&recvMsg[1]);
				printGameState(gameState);
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(_tickRate_ms));
	}

	_isClosed = true;
}

void Client::writeLog(const std::string& msg) const
{
	std::cout << std::format("{:%F %T} ", std::chrono::system_clock::now()) << msg << std::endl;
}

void Client::printGameState(const GameState& gameState) const
{
	std::stringstream ss;
	ss << "turn: " << (gameState.turn == Side::white ? "white" : "black") << "\n";
	for (int i = boardWidth - 1; i >= 0; --i)
	{
		ss << std::to_string(i + 1) << " |";
		for (int j = 0; j < boardWidth; ++j)
		{
			Piece piece = gameState.pieces.at(i * boardWidth + j);
			switch (piece)
			{
			case Piece::none:
				ss << "  |";
				break;

			case Piece::w_king:
				ss << "WK|";
				break;
			case Piece::w_queen:
				ss << "WQ|";
				break;
			case Piece::w_bishop:
				ss << "WB|";
				break;
			case Piece::w_knight:
				ss << "WN|";
				break;
			case Piece::w_rook:
				ss << "WR|";
				break;
			case Piece::w_pawn:
				ss << "WP|";
				break;

			case Piece::b_king:
				ss << "BK|";
				break;
			case Piece::b_queen:
				ss << "BQ|";
				break;
			case Piece::b_bishop:
				ss << "BB|";
				break;
			case Piece::b_knight:
				ss << "BN|";
				break;
			case Piece::b_rook:
				ss << "BR|";
				break;
			case Piece::b_pawn:
				ss << "BP|";
				break;
			}

		}
		ss << "\n\n";
	}
	ss << "   a  b  c  d  e  f  g  h\n";
	std::cout << ss.str();
}
