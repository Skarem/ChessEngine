#include "assets.hpp"

namespace Graphics
{

Assets::Assets(const BoardLayout& layout) :
    m_layout(layout)
{

}

void Assets::LoadAssets()
{
    Image boardImage    = LoadImage("assets/Chess_Board.png");
    Image spritesImage  = LoadImage("assets/Chess_Sprites.png");

    boardTexture = LoadTextureFromImage(boardImage);

    
    for (size_t i = 0; i < Chess::Type::Count; i++) {
        piecesTextures[Chess::Side::White][i]  = CropTextureFromImage(spritesImage, i, 0);
        piecesTextures[Chess::Side::Black][i]   = CropTextureFromImage(spritesImage, i, 1);
    }

    cursorTextures[CursorState::Hovered] = CropTextureFromImage(spritesImage, Chess::Type::Count, 0);
    cursorTextures[CursorState::Clicked] = CropTextureFromImage(spritesImage, Chess::Type::Count, 1);

    UnloadImage(boardImage);
    UnloadImage(spritesImage);
};

void Assets::UnloadAssets()
{
    UnloadTexture(boardTexture);
    for (size_t i = 0; i < Chess::Type::Count; i++) {
        UnloadTexture(piecesTextures[0][i]);
        UnloadTexture(piecesTextures[1][i]);
    }
    for (size_t i = 0; i < CursorState::Count; i++) {
        UnloadTexture(cursorTextures[i]);
    }
}

Texture2D Assets::CropTextureFromImage(Image sourceImage, const float x, const float y)
{
    Image frame = ImageCopy(sourceImage);
    ImageCrop(&frame, { x * m_layout.RAW_CELL_SIZE, y * m_layout.RAW_CELL_SIZE, 
        static_cast<float>(m_layout.RAW_CELL_SIZE), static_cast<float>(m_layout.RAW_CELL_SIZE) });
    Texture2D texture = LoadTextureFromImage(frame);
    UnloadImage(frame);
    return texture;
}

Vector2 Assets::GetBoardSize()
{ 
    return { 
        static_cast<float>(boardTexture.width),
        static_cast<float>(boardTexture.height)
    };
}

};