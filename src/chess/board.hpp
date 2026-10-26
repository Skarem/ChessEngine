#pragma once

#include "types.hpp"

#include <bit>
#include <cstdio>
#include <print>

namespace Chess {

using Bitboard = uint64_t;

struct Board
{
    Bitboard pieces[Side::Count][Type::Count] {};
    Bitboard occupancy[Side::Count] {};
};

namespace BB
{
    constexpr Bitboard FileA = 0x0101010101010101;
    constexpr Bitboard FileB = 0x0202020202020202;
    constexpr Bitboard FileG = 0x4040404040404040;
    constexpr Bitboard FileH = 0x8080808080808080;
    constexpr Bitboard Rank1 = 0x00000000000000FF;
    constexpr Bitboard Rank3 = 0x0000000000FF0000;
    constexpr Bitboard Rank6 = 0x0000FF0000000000;
    constexpr Bitboard Rank8 = 0xFF00000000000000;

    constexpr Bitboard FileAB  = FileA | FileB;
    constexpr Bitboard FileGH  = FileG | FileH;
    constexpr Bitboard Borders = FileA | FileH | Rank1 | Rank8;

    constexpr Bitboard SideOccupancy(const Bitboard pieces[Type::Count]) {
        return pieces[Type::Pawn] | pieces[Type::Knight] | pieces[Type::Bishop] |
               pieces[Type::Rook] | pieces[Type::Queen]  | pieces[Type::King];
    }

    constexpr Bitboard FullOccupancy(const Bitboard pieces[Side::Count][Type::Count]) {
        return SideOccupancy(pieces[Side::White]) | SideOccupancy(pieces[Side::Black]);
    }

    constexpr Bitboard Create(Square s)
    {
        return Bitboard{1} << s.Index();
    }

    constexpr void Set(Bitboard& board, Square s)
    {
        board |= Create(s);
    }

    constexpr void Clear(Bitboard& board, Square s)
    {
        board &= ~Create(s);
    }

    constexpr bool Test(Bitboard board, Square s)
    {
        return (board & Create(s)) != 0;
    }

    constexpr size_t Count(Bitboard board)
    {
        return std::popcount(board);
    }

    constexpr Square LSB(Bitboard board)
    {
        return static_cast<Square>(std::countr_zero(board));
    }

    constexpr Square PopLSB(Bitboard& board)
    {
        Square s = LSB(board);
        board &= board - 1;
        return s;
    }

    constexpr void Print(const Bitboard& board) {
        std::println("  a b c d e f g h");
        for (int rank = BoardSide - 1; rank >= 0; rank--) {
            std::printf("%d ", rank + 1);
            for (int file = 0; file < BoardSide; file++) {
                std::printf("%c ", BB::Test(board, Square(file, rank)) ? 'X' : '.');
            }
            std::println();
        }
        std::println();
    }

    constexpr Bitboard N(Bitboard b)  { return b << 8; }
    constexpr Bitboard S(Bitboard b)  { return b >> 8; }
    constexpr Bitboard E(Bitboard b)  { return (b << 1) & ~FileA; }
    constexpr Bitboard W(Bitboard b)  { return (b >> 1) & ~FileH; }

    constexpr Bitboard NE(Bitboard b) { return (b << 9) & ~FileA; }
    constexpr Bitboard NW(Bitboard b) { return (b << 7) & ~FileH; }
    constexpr Bitboard SE(Bitboard b) { return (b >> 7) & ~FileA; }
    constexpr Bitboard SW(Bitboard b) { return (b >> 9) & ~FileH; }
}

};