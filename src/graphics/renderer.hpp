#pragma once

#include "chess/board.hpp"
#include "graphics/layout.hpp"
#include "graphics/assets.hpp"

#include <vector>

namespace Graphics
{

class Renderer
{
public:
    Renderer(const BoardLayout& layout);
    ~Renderer();

    void Draw(const Chess::Board& board, Chess::Square hovered, Chess::Square selected,
              const std::vector<Chess::Move>& legalMoves, bool promotionPending, bool inCheck);

private:
    void DrawBoard();
    void DrawPieces(const Chess::Board& board);
    void DrawPiecesType(uint8_t color, uint8_t type, Chess::Bitboard pieces);
    void DrawPiece(uint8_t color, uint8_t type, Chess::Square sq);
    void DrawCursor(uint8_t state, Chess::Square sq);
    void DrawHighlightCell(Chess::Square sq);
    void DrawScaledTexture(const Texture2D& texture, float x, float y);

    BoardLayout m_layout;
    Assets m_assets;
};

};