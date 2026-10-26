#pragma once

#include "chess/chess.hpp"
#include "engine/engine.hpp"
#include "graphics/layout.hpp"
#include "graphics/renderer.hpp"
#include "input/input.hpp"

namespace App
{

class Game
{
public:
    explicit Game(int scale);
    void Run();

private:
    void HandleInput(const Input::InputFrame& input);
    bool IsOwnPiece(Chess::Square square) const;
    void TryMove(Chess::Square destination, uint8_t promotion = Chess::Type::None);
    void ClearSelection();
    
    const int EngineDepth = 4;

    Graphics::BoardLayout m_layout;
    Chess::Chess          m_chess;
    Chess::Engine        m_engine;
    Input::Input          m_input;
    Graphics::Renderer    m_renderer;

    Chess::Square m_selected = Chess::NoSquare;
    Chess::Square m_pendingPromotion = Chess::NoSquare;
};

};