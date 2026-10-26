#pragma once

#include "board.hpp"

namespace Chess
{

namespace Castling
{
    constexpr uint8_t WhiteKingside  = 1 << 0;
    constexpr uint8_t WhiteQueenside = 1 << 1;
    constexpr uint8_t BlackKingside  = 1 << 2;
    constexpr uint8_t BlackQueenside = 1 << 3;
    constexpr uint8_t FullRights     = 0b1111;
    constexpr uint8_t WhiteRights    = WhiteKingside | WhiteQueenside;
    constexpr uint8_t BlackRights    = BlackKingside | BlackQueenside;

    inline bool HasRights(uint8_t rights, uint8_t mask) { return (rights & mask) != 0; }
    inline void Set(uint8_t& rights, uint8_t mask)      { rights |= mask; }
    inline void Clear(uint8_t& rights, uint8_t mask)    { rights &= ~mask; }
};

struct Position
{
    Board board;

    uint8_t sideToMove = Side::White;

    uint8_t castlingRights  = 0;
    Square  enPassantTarget = NoSquare;

    uint16_t halfmoveClock  = 0;
    uint16_t fullmoveNum    = 1;
};

};