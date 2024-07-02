#pragma once

#include <IlkpChessCommon/chess.h>

#include <cstdint>
#include <array>
#include <vector>
#include <random>

using GameHash = uint64_t;

class ZobristTable
{
public:
	ZobristTable();

	inline GameHash turn(Side side) const { return _turn.at(static_cast<uTypeSide>(side)); };
	inline GameHash longCastle(Side side) const { return _longCastle.at(static_cast<uTypeSide>(side)); };
	inline GameHash shortCastle(Side side) const { return _shortCastle.at(static_cast<uTypeSide>(side)); };
	inline GameHash enPassant(Side side, size_t index) const { return _endPassant.at(static_cast<uTypeSide>(side))[index]; }
	inline GameHash enPassantAvailable(Side side, size_t index) const { return _enPassantAvailable.at(static_cast<uTypeSide>(side))[index]; };

	GameHash PieceHash(Piece piece, int x, int y) const;

private:
	static const int nPieceHashes = boardSize * static_cast<uTypePiece>(Piece::maxValue);

	std::array<GameHash, 2> _turn;
	std::array<GameHash, 2> _longCastle;
	std::array<GameHash, 2> _shortCastle;
	std::array<std::array<GameHash, boardWidth>, 2> _endPassant;
	std::array<std::array<GameHash, boardWidth>, 2> _enPassantAvailable;
	std::array<GameHash, nPieceHashes> _pieces;

	GameHash uniqueRandomGameHash(
		std::vector<GameHash>& usedValues,
		std::uniform_int_distribution<uint64_t>& dist,
		std::mt19937_64& engine) const;
};

struct MoveHash
{
	std::vector<GameHash> remove;
	std::vector<GameHash> add;
};

GameHash hashGameState(const GameState& gameState, const ZobristTable& zobristTable);

GameHash applyMoveHash(const GameHash gameHash, const MoveHash move);