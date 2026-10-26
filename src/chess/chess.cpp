#include "chess.hpp"

namespace Chess 
{

Chess::Chess()
{
    Reset();
}

void Chess::Reset()
{
    m_position = FEN::LoadFEN(InitialFenPosition);
    m_history.clear();
    RefreshLegalMoves();
}

void Chess::RefreshLegalMoves()
{
    m_legalMoves = MoveGen::LegalMoves(m_position);
    m_inCheck    = MoveGen::IsInCheck(m_position, m_position.sideToMove);
}


bool Chess::PlayMove(const Move& requested)
{
    const auto it = std::find_if(m_legalMoves.begin(), m_legalMoves.end(), [&](const Move& m) {
        return m.from.Index() == requested.from.Index() && m.to.Index() == requested.to.Index() &&
               m.type == requested.type && m.promotion == requested.promotion;
    });

    if (it == m_legalMoves.end()) return false;

    const Move selected = *it;
    UndoState undo = MoveGen::MakeMove(m_position, selected);

    m_history.push_back({ selected, undo });
    RefreshLegalMoves();

    return true;
}

bool Chess::UndoLastMove()
{
    if (m_history.empty()) return false;

    const HistoryEntry entry = m_history.back();

    MoveGen::UnmakeMove(m_position, entry.move, entry.undo);
    m_history.pop_back();
    RefreshLegalMoves();

    return true;
}

};