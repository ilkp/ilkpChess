#include "zobrist.h"


ZobristTable::ZobristTable()
{
	std::random_device rd;
	std::mt19937_64 engine(rd());
	std::uniform_int_distribution<uint64_t> dist;
	std::vector<GameHash> usedValues;
	size_t whiteIndex = static_cast<uTypeSide>(Side::white);
	size_t blackIndex = static_cast<uTypeSide>(Side::black);
	_turn[whiteIndex] = uniqueRandomGameHash(usedValues, dist, engine);
	_turn[blackIndex] = uniqueRandomGameHash(usedValues, dist, engine);
	_longCastle[whiteIndex] = uniqueRandomGameHash(usedValues, dist, engine);
	_longCastle[blackIndex] = uniqueRandomGameHash(usedValues, dist, engine);
	_shortCastle[whiteIndex] = uniqueRandomGameHash(usedValues, dist, engine);
	_shortCastle[blackIndex] = uniqueRandomGameHash(usedValues, dist, engine);

	for (GameHash& piece : _pieces)
		piece = uniqueRandomGameHash(usedValues, dist, engine);
	for (GameHash& enPassant : _endPassant.at(whiteIndex))
		enPassant = uniqueRandomGameHash(usedValues, dist, engine);
	for (GameHash& enPassant : _endPassant.at(blackIndex))
		enPassant = uniqueRandomGameHash(usedValues, dist, engine);
	for (GameHash& enPassant : _enPassantAvailable.at(whiteIndex))
		enPassant = uniqueRandomGameHash(usedValues, dist, engine);
	for (GameHash& enPassant : _enPassantAvailable.at(blackIndex))
		enPassant = uniqueRandomGameHash(usedValues, dist, engine);
}

GameHash ZobristTable::PieceHash(Piece piece, int x, int y) const
{
	const size_t index = y * static_cast<uTypePiece>(Piece::maxValue) * boardWidth
		+ x * static_cast<uTypePiece>(Piece::maxValue)
		+ static_cast<uTypePiece>(piece);
	return _pieces.at(index);
}

GameHash ZobristTable::uniqueRandomGameHash(std::vector<GameHash>& usedValues, std::uniform_int_distribution<uint64_t>& dist, std::mt19937_64& engine) const
{
	GameHash value;
	do
	{
		value = dist(engine);
	} while (std::find(usedValues.begin(), usedValues.end(), value) != usedValues.end());
	usedValues.push_back(value);
	return value;
}

GameHash hashGameState(const GameState& gameState, const ZobristTable& zobristTable)
{
	GameHash gameHash = 0;

	gameHash ^= zobristTable.turn(gameState.turn);

	constexpr uTypeSide whiteIndex = static_cast<uTypeSide>(Side::white);
	constexpr uTypeSide blackIndex = static_cast<uTypeSide>(Side::black);

	if (gameState.enPassant.at(whiteIndex) != -1)
		gameHash ^= zobristTable.enPassant(Side::white, gameState.enPassant.at(whiteIndex));
	if (gameState.enPassant.at(blackIndex) != -1)
		gameHash ^= zobristTable.enPassant(Side::black, gameState.enPassant.at(whiteIndex));
	if (gameState.longCastleAvailable.at(whiteIndex))
		gameHash ^= zobristTable.longCastle(Side::white);
	if (gameState.longCastleAvailable.at(blackIndex))
		gameHash ^= zobristTable.longCastle(Side::black);
	if (gameState.shortCastleAvailable.at(whiteIndex))
		gameHash ^= zobristTable.shortCastle(Side::white);
	if (gameState.shortCastleAvailable.at(blackIndex))
		gameHash ^= zobristTable.shortCastle(Side::black);

	for (int i = 0; i < boardWidth; ++i)
	{
		if (gameState.enPassantAvailable.at(whiteIndex).at(i))
			gameHash ^= zobristTable.enPassantAvailable(Side::white, i);
		if (gameState.enPassantAvailable.at(blackIndex).at(i))
			gameHash ^= zobristTable.enPassantAvailable(Side::black, i);
	}

	for (int y = 0; y < boardWidth; ++y)
		for (int x = 0; x < boardWidth; ++x)
			gameHash ^= zobristTable.PieceHash(gameState.pieces.at(y * boardWidth + x), x, y);

	return gameHash;
}

GameHash applyMoveHash(const GameHash gameHash, const MoveHash move)
{
	GameHash nextHash = gameHash;
	for (const GameHash remove : move.remove)
		nextHash ^= remove;
	for (const GameHash add : move.add)
		nextHash ^= add;
	return nextHash;
}