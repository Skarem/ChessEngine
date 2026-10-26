#include "game.hpp"

#include "chess/fen.hpp"
#include "chess/perft.hpp"

namespace App {

using Chess::Square;
using Chess::Move;

Game::Game(int scale) :
    m_layout(scale),
    m_chess(),
    m_input(m_layout),
    m_renderer(m_layout)
{
    // Chess::Perft perft;
    // perft.TestBatchPositions();
}

void Game::Run()
{
    while (!WindowShouldClose()) {
        const Input::InputFrame frame = m_input.Poll();

        HandleInput(frame);

        m_renderer.Draw(m_chess.GetBoard(), frame.hovered, m_selected,
                        m_chess.GetLegalMoves(), m_pendingPromotion.Index() != Chess::NoSquare,
                        m_chess.IsInCheck());

        if (m_chess.GetPosition().sideToMove == Chess::Side::Black &&
            !m_chess.GetLegalMoves().empty()) {
            auto bestMove = m_engine.FindBestMove(m_chess.GetPosition(), EngineDepth);
            if (bestMove)
            m_chess.PlayMove(*bestMove);
        }
    }
}

bool Game::IsOwnPiece(Square square) const
{
    if (square.Index() >= Chess::NoSquare) return false;

    uint8_t color = m_chess.GetPosition().sideToMove;
    return Chess::BB::Test(m_chess.GetBoard().occupancy[color], square);
}

void Game::ClearSelection()
{
    m_selected = Chess::NoSquare;
    m_pendingPromotion = Chess::NoSquare;
}

void Game::TryMove(Square destination, uint8_t promotion)
{
    for (const Move& move : m_chess.GetLegalMoves()) {
        if (move.from.Index() != m_selected.Index() || move.to.Index() != destination.Index())
            continue;

        if (move.type == Chess::MoveType::Promotion && promotion == Chess::Type::None) {
            m_pendingPromotion = destination;
            return;
        }

        if (move.type == Chess::MoveType::Promotion && move.promotion != promotion)
            continue;

        if (m_chess.PlayMove(move))
            ClearSelection();

        return;
    }
}

void Game::HandleInput(const Input::InputFrame& event)
{
    if (event.undo) {
        m_chess.UndoLastMove();
        ClearSelection();
        return;
    }

    if (event.cancel) {
        ClearSelection();
        return;
    }

    if (m_pendingPromotion.Index() != Chess::NoSquare) {
        if (event.promotionPiece != Chess::Type::None)
            TryMove(m_pendingPromotion, event.promotionPiece);
        return;
    }

    if (event.clicked.Index() == Chess::NoSquare || m_chess.GetLegalMoves().empty())
        return;

    if (event.clicked.Index() == m_selected.Index()) {
        ClearSelection();
        return;
    }

    if (IsOwnPiece(event.clicked)) {
        m_selected = event.clicked;
        return;
    }

    if (m_selected.Index() != Chess::NoSquare)
        TryMove(event.clicked);
}

}