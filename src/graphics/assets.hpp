#pragma once

#include "raylib.h"

#include "chess/board.hpp"
#include "layout.hpp"

#include <cstdint>

namespace Graphics
{

struct CursorState
{ 
    static const uint8_t Hovered   = 0;
    static const uint8_t Clicked   = 1;
    static const uint8_t Count     = 2;
};

class Assets
{
public:
    Assets(const BoardLayout& layout);
    void LoadAssets();
    void UnloadAssets();
    Vector2 GetBoardSize();

    Texture2D boardTexture {};
    Texture2D piecesTextures[Chess::Side::Count][Chess::Type::Count] {};
    Texture2D cursorTextures[CursorState::Count] {};

private:
    Texture2D CropTextureFromImage(Image sourceImage, const float x, const float y);
    BoardLayout m_layout;
};

};