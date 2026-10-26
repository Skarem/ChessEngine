#pragma once

#include "position.hpp"
#include "movegen.hpp"
#include "fen.hpp"

#include <vector>

namespace Chess
{

class Chess
{
public:
    Chess();

    const Board& GetBoard() const { return m_position.board; }
    const Position& GetPosition() const { return m_position; }
    const std::vector<Move>& GetLegalMoves() const { return m_legalMoves; }

    bool PlayMove(const Move& move);
    bool UndoLastMove();
    bool IsInCheck() const { return m_inCheck; }
    void Reset();

private:
    void RefreshLegalMoves();

    struct HistoryEntry
    {
        Move move;
        UndoState undo;
    };

    Position m_position {};
    std::vector<Move> m_legalMoves;
    std::vector<HistoryEntry> m_history;
    bool m_inCheck = false;

    const std::string InitialFenPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
};

};