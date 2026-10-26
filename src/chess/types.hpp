#pragma once

#include <cstdint>

namespace Chess
{

constexpr uint8_t BoardSide   = 8;
constexpr uint8_t SquareCount = BoardSide * BoardSide;
constexpr uint8_t NoSquare    = SquareCount;

struct Square
{
    uint8_t sq;

    constexpr Square(uint8_t value) { sq = value; }
    constexpr Square(uint8_t file, uint8_t rank) { sq = static_cast<uint8_t>(file + BoardSide * rank); }

    constexpr uint8_t Index() const { return sq; }
    constexpr uint8_t File() const { return sq % BoardSide; }
    constexpr uint8_t Rank() const { return sq / BoardSide; }
};

namespace Side
{
    constexpr uint8_t White = 0;
    constexpr uint8_t Black = 1;
    constexpr uint8_t Count = 2;

    constexpr uint8_t Other(const uint8_t side) { return side ^ 1; }
};

namespace Type
{
    constexpr uint8_t Pawn    = 0;
    constexpr uint8_t Knight  = 1;
    constexpr uint8_t Bishop  = 2;
    constexpr uint8_t Rook    = 3;
    constexpr uint8_t Queen   = 4;
    constexpr uint8_t King    = 5;
    constexpr uint8_t Count   = 6;
    constexpr uint8_t None    = 7;
};

enum class MoveType : uint8_t
{
    Normal,
    Castling,
    EnPassant,
    Promotion
};

struct Move
{
    Square from;
    Square to;
    MoveType type      = MoveType::Normal;
    uint8_t  promotion = Type::None;
};

struct UndoState
{
    uint8_t  capturedType = Type::None;
    uint8_t  prevCastlingRights;
    Square   prevEnPassantTarget = NoSquare;
    uint16_t prevHalfmoveClock;
};

};