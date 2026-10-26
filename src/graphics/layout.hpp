#pragma once

#include "raylib.h"
#include "chess/types.hpp"

#include <cmath>

namespace Graphics
{

struct BoardLayout
{
    explicit BoardLayout(int scale) : 
        SCALE(scale),
        SCALED_MARGIN(scale * RAW_MARGIN),
        SCALED_CELL_SIZE(scale * RAW_CELL_SIZE)
    {

    }

    Chess::Square ScreenToSquare(float x, float y) const
    {
        int file = static_cast<int>(floor((x - SCALED_MARGIN) / SCALED_CELL_SIZE));
        int rank = static_cast<int>(floor((y - SCALED_MARGIN) / SCALED_CELL_SIZE));
        
        if (file < 0 || file >= Chess::BoardSide || rank < 0 || rank >= Chess::BoardSide) {
            return Chess::NoSquare;
        }

        return Chess::Square(file, Chess::BoardSide - 1 - rank);
    }

    Vector2 GridToScreen(Chess::Square square) const
    { 
        return {
            static_cast<float>(square.File() * SCALED_CELL_SIZE + SCALED_MARGIN),
            static_cast<float>((Chess::BoardSide - 1 - square.Rank()) * SCALED_CELL_SIZE + SCALED_MARGIN)
        };
    }

    const int SCALE = 0;
    const int RAW_MARGIN    = 2;
    const int RAW_CELL_SIZE = 16;
    const int SCALED_MARGIN;
    const int SCALED_CELL_SIZE;
};

};