#include "chess.h"


double evaluate(const GameState& gameState)
{
	double value = 0;
	for (Piece piece : gameState.pieces)
	{
		switch (piece)
		{
		case Piece::w_queen:
			value += 9;
			break;
		case Piece::w_bishop: case Piece::w_knight:
			value += 3;
			break;
		case Piece::w_rook:
			value += 5;
			break;
		case Piece::w_pawn:
			value += 1;
			break;
		case Piece::b_queen:
			value -= 9;
			break;
		case Piece::b_bishop: case Piece::b_knight:
			value -= 3;
			break;
		case Piece::b_rook:
			value -= 5;
			break;
		case Piece::b_pawn:
			value -= 1;
			break;
		default:
			break;
		}
	}
	return value;
}

Side evaluateWinner(const GameState& gameState)
{
	return Side::none;
}

bool moveIsLegal(const Move& move, const GameState& gameState)
{
	return true;
}

GameState applyMove(const Move& move, const GameState& gameState, bool applyTurn)
{
	GameState nextGameState = gameState;
	const Side nextTurn = gameState.turn == Side::white ? Side::black : Side::white;
	const size_t fromIndex = move.yFrom * boardWidth + move.xFrom;
	const size_t toIndex = move.yTo * boardWidth + move.xTo;
	const Piece fromPiece = gameState.pieces.at(fromIndex);

	const bool isLongCastle =
		fromPiece == Piece::w_king || fromPiece == Piece::b_king &&
		move.xFrom == 4 &&
		move.xTo == 2;
	const bool isShortCastle =
		fromPiece == Piece::w_king || fromPiece == Piece::b_king &&
		move.xFrom == 4 &&
		move.xTo == 6;
	const bool isEnpassant =
		fromPiece == Piece::w_pawn && move.yTo == move.yFrom + 2 ||
		fromPiece == Piece::b_pawn && move.yTo == move.yFrom - 2;

	if (isEnpassant)
	{
		nextGameState.enPassant[static_cast<uTypeSide>(nextTurn)] = move.xFrom;
		nextGameState.enPassant[static_cast<uTypeSide>(gameState.turn)] = -1;
		nextGameState.enPassantAvailable.at(static_cast<uTypeSide>(gameState.turn))[move.xFrom] = false;
	}
	else
	{
		nextGameState.enPassant[static_cast<uTypeSide>(Side::white)] = -1;
		nextGameState.enPassant[static_cast<uTypeSide>(Side::black)] = -1;
	}

	nextGameState.pieces[fromIndex] = Piece::none;
	nextGameState.pieces[toIndex] = fromPiece;

	if (isLongCastle)
	{
		nextGameState.longCastleAvailable[static_cast<uTypeSide>(gameState.turn)] = false;
		nextGameState.pieces[fromIndex - 1] = gameState.turn == Side::white ? Piece::w_rook : Piece::b_rook;
		nextGameState.pieces[fromIndex - 4] = Piece::none;
	}
	if (isShortCastle)
	{
		nextGameState.shortCastleAvailable[static_cast<uTypeSide>(gameState.turn)] = false;
		nextGameState.pieces[fromIndex + 1] = gameState.turn == Side::white ? Piece::w_rook : Piece::b_rook;
		nextGameState.pieces[fromIndex + 3] = Piece::none;
	}

	if (applyTurn)
		nextGameState.turn = nextTurn;
	return nextGameState;
}
