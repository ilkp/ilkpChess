#pragma once

#include <vector>
#include <array>

constexpr uint64_t boardWidth = 8;
constexpr uint64_t boardSize = boardWidth * boardWidth;

enum class Side
{
	white,
	black,
	nSides,
	none
};
typedef std::underlying_type<Side>::type uTypeSide;

enum class Piece : char
{
	none,
	w_king,
	w_queen,
	w_bishop,
	w_knight,
	w_rook,
	w_pawn,
	b_king,
	b_queen,
	b_bishop,
	b_knight,
	b_rook,
	b_pawn,
	maxValue
};
typedef std::underlying_type<Piece>::type uTypePiece;

struct GameState
{
	Side turn;
	std::array<bool, static_cast<uTypeSide>(Side::nSides)> longCastleAvailable;
	std::array<bool, static_cast<uTypeSide>(Side::nSides)> shortCastleAvailable;
	std::array<char, static_cast<uTypeSide>(Side::nSides)> enPassant;
	std::array<std::array<bool, boardWidth>, static_cast<uTypeSide>(Side::nSides)> enPassantAvailable;
	std::array<Piece, boardSize> pieces;

	inline static GameState defaultStartingState()
	{
		constexpr uTypeSide whiteIndex = static_cast<uTypeSide>(Side::white);
		constexpr uTypeSide blackIndex = static_cast<uTypeSide>(Side::black);

		GameState gameState{};
		gameState.turn = Side::white;

		gameState.enPassant.at(whiteIndex) = -1;
		gameState.enPassant.at(blackIndex) = -1;
		gameState.longCastleAvailable.at(whiteIndex) = -1;
		gameState.longCastleAvailable.at(blackIndex) = -1;
		gameState.shortCastleAvailable.at(whiteIndex) = -1;
		gameState.shortCastleAvailable.at(blackIndex) = -1;

		for (size_t i = 0; i < boardWidth; ++i)
		{
			gameState.enPassantAvailable[whiteIndex][i] = true;
			gameState.enPassantAvailable[blackIndex][i] = true;
		}

		for (size_t i = 0; i < boardWidth; ++i)
		{
			gameState.pieces[boardWidth + i] = Piece::w_pawn;
			gameState.pieces[6 * boardWidth + i] = Piece::b_pawn;
		}

		gameState.pieces[0] = Piece::w_rook;
		gameState.pieces[1] = Piece::w_knight;
		gameState.pieces[2] = Piece::w_bishop;
		gameState.pieces[3] = Piece::w_queen;
		gameState.pieces[4] = Piece::w_king;
		gameState.pieces[5] = Piece::w_bishop;
		gameState.pieces[6] = Piece::w_knight;
		gameState.pieces[7] = Piece::w_rook;

		const uint64_t blackPiecesRow = 7 * boardWidth;
		gameState.pieces[blackPiecesRow + 0] = Piece::b_rook;
		gameState.pieces[blackPiecesRow + 1] = Piece::b_knight;
		gameState.pieces[blackPiecesRow + 2] = Piece::b_bishop;
		gameState.pieces[blackPiecesRow + 3] = Piece::b_queen;
		gameState.pieces[blackPiecesRow + 4] = Piece::b_king;
		gameState.pieces[blackPiecesRow + 5] = Piece::b_bishop;
		gameState.pieces[blackPiecesRow + 6] = Piece::b_knight;
		gameState.pieces[blackPiecesRow + 7] = Piece::b_rook;

		for (int y = 2; y < 6; ++y)
			for (int x = 0; x < boardWidth; ++x)
				gameState.pieces[y * boardWidth + x] = Piece::none;

		return gameState;
	}
};

struct GameFormat
{
	int time_s = 600;
	int timeControl_s = 0;
	int timeControlTurn = 0;
	int increment_s = 0;
	int incrementBeginTurn = 0;
};

struct Move
{
	int xFrom;
	int yFrom;
	int xTo;
	int yTo;
};

double evaluate(const GameState& gameState);

Side evaluateWinner(const GameState& gameState);

bool moveIsLegal(const Move& move, const GameState& gameState);

GameState applyMove(const Move& move, const GameState& gameState, bool applyTurn = true);
