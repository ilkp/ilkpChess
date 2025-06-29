#pragma once
#include "socket.h"
#include "chess.h"

struct Player
{
	Socket socket;
	std::string ip;
	Side playingAs;
};