#include "chess.h"

GameState GameState::defaultStartingState()
{
	constexpr PlayerUt whiteIndex = static_cast<PlayerUt>(Side::white);
	constexpr PlayerUt blackIndex = static_cast<PlayerUt>(Side::black);

	GameState gameState{};
	gameState.turn = Side::white;

	gameState.enPassants.at(whiteIndex) = -1;
	gameState.enPassants.at(blackIndex) = -1;
	gameState.longCastlesAvailable.at(whiteIndex) = -1;
	gameState.longCastlesAvailable.at(blackIndex) = -1;
	gameState.shortCastlesAvailable.at(whiteIndex) = -1;
	gameState.shortCastlesAvailable.at(blackIndex) = -1;

	for (size_t i = 0; i < boardWidth; ++i)
	{
		gameState.enPassantsAvailable[whiteIndex][i] = true;
		gameState.enPassantsAvailable[blackIndex][i] = true;
	}

	for (size_t i = 0; i < boardWidth; ++i)
	{
		gameState.pieces[boardWidth + i] = Piece::wPawn;
		gameState.pieces[6 * boardWidth + i] = Piece::bPawn;
	}

	gameState.pieces[0] = Piece::wRook;
	gameState.pieces[1] = Piece::wKnight;
	gameState.pieces[2] = Piece::wBishop;
	gameState.pieces[3] = Piece::wQueen;
	gameState.pieces[4] = Piece::wKing;
	gameState.pieces[5] = Piece::wBishop;
	gameState.pieces[6] = Piece::wKnight;
	gameState.pieces[7] = Piece::wRook;

	const int blackPiecesRow = 7 * boardWidth;
	gameState.pieces[blackPiecesRow + 0] = Piece::bRook;
	gameState.pieces[blackPiecesRow + 1] = Piece::bKnight;
	gameState.pieces[blackPiecesRow + 2] = Piece::bBishop;
	gameState.pieces[blackPiecesRow + 3] = Piece::bQueen;
	gameState.pieces[blackPiecesRow + 4] = Piece::bKing;
	gameState.pieces[blackPiecesRow + 5] = Piece::bBishop;
	gameState.pieces[blackPiecesRow + 6] = Piece::bKnight;
	gameState.pieces[blackPiecesRow + 7] = Piece::bRook;

	for (int y = 2; y < 6; ++y)
		for (int x = 0; x < boardWidth; ++x)
			gameState.pieces[y * boardWidth + x] = Piece::none;

	return gameState;
}
