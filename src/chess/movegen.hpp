#pragma once

#include "position.hpp"

#include <vector>

namespace Chess
{

class MoveGen
{
public:
    MoveGen() = delete;

    static UndoState MakeMove(Position& position, const Move& move);
    static void UnmakeMove(Position& position, const Move& move, const UndoState& undoState);

    static std::vector<Move> PseudoLegalMoves(const Position& position);
    static void PseudoLegalMoves(const Position& position, std::vector<Move>& moves);
    static std::vector<Move> LegalMoves(const Position& position);
    static void LegalMoves(Position& position, std::vector<Move>& moves);

    static bool IsInCheck(const Position& position, uint8_t color);
    static uint8_t GetType(const Board& board, Square square, uint8_t color);

private:
    static void PawnAttacks  (std::vector<Move>& moves, Bitboard pawns,   Bitboard occupancyOwn, Bitboard occupancyOther, Square enPassantTarget, uint8_t color);
    static void KnightAttacks(std::vector<Move>& moves, Bitboard knights, Bitboard occupancyOwn);
    static void BishopAttacks(std::vector<Move>& moves, Bitboard bishops, Bitboard occupancyOwn, Bitboard occupancyOther);
    static void RookAttacks  (std::vector<Move>& moves, Bitboard rooks,   Bitboard occupancyOwn, Bitboard occupancyOther);
    static void QueenAttacks (std::vector<Move>& moves, Bitboard queens,  Bitboard occupancyOwn, Bitboard occupancyOther);
    static void KingAttacks  (std::vector<Move>& moves, Bitboard king,    Bitboard occupancyOwn);

    static void AddPawnMove  (std::vector<Move>& moves, Square from, Square to, uint8_t color);
    static void CastlingMoves(std::vector<Move>& moves, const Position& position, uint8_t color);

    static bool IsSquareAttacked(const Position& position, Square square, uint8_t byColor);
    static bool IsSquareAttacked(const Position& position, Square square, uint8_t byColor, Bitboard occupancy); 
};

};