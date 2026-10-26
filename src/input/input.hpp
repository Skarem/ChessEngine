#pragma once

#include "chess/types.hpp"
#include "graphics/layout.hpp"

#include "raylib.h"

namespace Input
{

struct InputFrame
{
    Chess::Square hovered = Chess::NoSquare;
    Chess::Square clicked = Chess::NoSquare;

    uint8_t promotionPiece = Chess::Type::None;
    bool cancel = false;
    bool undo = false;
};

class Input
{
public:
    explicit Input(const Graphics::BoardLayout& layout) : m_layout(layout) {}
    
    InputFrame Poll() const
    {
        InputFrame event;

        const Vector2 mouse = GetMousePosition();
        event.hovered = m_layout.ScreenToSquare(mouse.x, mouse.y);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        event.clicked = event.hovered;

        if (IsKeyPressed(KEY_Q))      event.promotionPiece = Chess::Type::Queen;
        else if (IsKeyPressed(KEY_R)) event.promotionPiece = Chess::Type::Rook;
        else if (IsKeyPressed(KEY_B)) event.promotionPiece = Chess::Type::Bishop;
        else if (IsKeyPressed(KEY_N)) event.promotionPiece = Chess::Type::Knight;

        event.cancel = IsKeyPressed(KEY_ESCAPE);
        event.undo = IsKeyPressed(KEY_BACKSPACE);

        return event;
    }

private:
    const Graphics::BoardLayout& m_layout;
};

};