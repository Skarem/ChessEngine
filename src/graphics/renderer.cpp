#include "renderer.hpp"

namespace Graphics
{

Renderer::Renderer(const BoardLayout& layout) :
    m_layout(layout),
    m_assets(layout)
{
    SetTraceLogLevel(LOG_NONE);
    InitWindow(100, 100, "Chess");
    SetExitKey(0); // Escape cancels a selection instead of closing the window.

    m_assets.LoadAssets();

    Vector2 windowSize = m_assets.GetBoardSize();
    SetWindowSize(static_cast<int>(windowSize.x * m_layout.SCALE), static_cast<int>(windowSize.y * m_layout.SCALE));
    
    SetTargetFPS(60);
}

Renderer::~Renderer()
{
    m_assets.UnloadAssets();
    CloseWindow();
}

void Renderer::Draw(const Chess::Board& board, Chess::Square hovered, Chess::Square selected,
              const std::vector<Chess::Move>& legalMoves, bool promotionPending, bool inCheck)
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawBoard();

    if (selected.Index() != Chess::NoSquare) {
        for (const Chess::Move& move : legalMoves) {
            if (move.from.Index() == selected.Index())
                DrawHighlightCell(move.to);
        }
    }

    DrawPieces(board);

    if (hovered.Index() != Chess::NoSquare && hovered.Index() != selected.Index())
        DrawCursor(CursorState::Hovered, hovered);

    if (selected.Index() != Chess::NoSquare)
        DrawCursor(CursorState::Clicked, selected);
    
    if (promotionPending) {
        DrawRectangle(0, 0, GetScreenWidth(), 32, Color{0, 0, 0, 210});
        DrawText("Promote: Q / R / B / N    Esc: cancel", 8, 8, 16, WHITE);
    }
    else if (legalMoves.empty()) {
        DrawRectangle(0, 0, GetScreenWidth(), 32, Color{0, 0, 0, 210});
        DrawText(inCheck ? "Checkmate" : "Stalemate", 8, 8, 20, WHITE);
    }

    EndDrawing();
}

void Renderer::DrawBoard()
{
    DrawScaledTexture(m_assets.boardTexture, 0, 0);
}

void Renderer::DrawPieces(const Chess::Board& board)
{
    for (uint8_t color = 0; color < Chess::Side::Count; color++) {
        for (uint8_t type = 0; type < Chess::Type::Count; type++) {
            DrawPiecesType(color, type, board.pieces[color][type]);
        }
    }
}

void Renderer::DrawPiecesType(uint8_t color, uint8_t type, Chess::Bitboard pieces)
{
    while (pieces) {
        Chess::Square sq = Chess::BB::PopLSB(pieces);
        DrawPiece(color, type, sq);
    }
}

void Renderer::DrawPiece(uint8_t color, uint8_t type, Chess::Square sq)
{
    Vector2 pos = m_layout.GridToScreen(sq);
    DrawScaledTexture(m_assets.piecesTextures[color][type], pos.x, pos.y);
}

void Renderer::DrawCursor(uint8_t state, Chess::Square sq)
{
    if (sq.Index() == Chess::NoSquare) return;
    Vector2 pos = m_layout.GridToScreen(sq);
    DrawScaledTexture(m_assets.cursorTextures[state], pos.x, pos.y);
}

void Renderer::DrawHighlightCell(Chess::Square sq)
{
    static const Color CELL_HIGHLIGHT_COLOR = { 255, 0, 0, 100 };
    Vector2 pos = m_layout.GridToScreen(sq);
    DrawRectangle(pos.x, pos.y, m_layout.SCALED_CELL_SIZE, m_layout.SCALED_CELL_SIZE, CELL_HIGHLIGHT_COLOR);
}

void Renderer::DrawScaledTexture(const Texture2D& texture, float x, float y)
{
    Rectangle src { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dst { x, y, (float)texture.width * m_layout.SCALE, (float)texture.height * m_layout.SCALE };
    DrawTexturePro(texture, src, dst, { 0, 0 }, 0, WHITE);
}

};