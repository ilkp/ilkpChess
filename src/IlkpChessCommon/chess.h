#pragma once

#include <vector>
#include <array>

constexpr int boardWidth = 8;
constexpr int boardSize = boardWidth * boardWidth;

enum class Side
{
	white,
	black,
	max,
	none
};
typedef std::underlying_type<Side>::type PlayerUt;

enum class Piece
{
	none,
	wKing,
	wQueen,
	wBishop,
	wKnight,
	wRook,
	wPawn,
	bKing,
	bQueen,
	bBishop,
	bKnight,
	bRook,
	bPawn,
	maxValue
};
typedef std::underlying_type<Piece>::type PieceUt;

struct GameState
{
	Side turn;
	std::array<bool, static_cast<PlayerUt>(Side::max)> longCastlesAvailable;
	std::array<bool, static_cast<PlayerUt>(Side::max)> shortCastlesAvailable;
	std::array<bool, static_cast<PlayerUt>(Side::max)> enPassants;
	std::array<std::array<bool, boardWidth>, static_cast<PlayerUt>(Side::max)> enPassantsAvailable;
	std::array<Piece, boardSize> pieces;

	static GameState defaultStartingState();
};

struct GameFormat
{
	int time_s = 600;
	int timeControl_s = 0;
	int timeControlTurn = 0;
	int increment_s = 0;
};

struct Move
{
	int xFrom;
	int yFrom;
	int xTo;
	int yTo;
};