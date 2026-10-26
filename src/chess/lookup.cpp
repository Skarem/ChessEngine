#include "lookup.hpp"

#include <random>

namespace Chess 
{
namespace Lookup
{

const AttackTables& Get()
{
    static const AttackTables tables;
    return tables;
}

AttackTables::AttackTables() :
    m_pawn  (InitPawnLookup()),
    m_knight(InitKnightLookup()),
    m_king  (InitKingLookup()),
    m_rook  (InitRookLookup()),
    m_bishop(InitBishopLookup())
{

}

Bitboard AttackTables::Pawn(uint8_t side, Square square) const
{
    return m_pawn[side][square.Index()];
}

Bitboard AttackTables::Knight(Square square) const
{
    return m_knight[square.Index()];
}

Bitboard AttackTables::King(Square square) const
{
    return m_king[square.Index()];
}

Bitboard AttackTables::Rook(Square square, Bitboard occupancy) const 
{
    const MagicBitboard& data = m_rook[square.Index()];
    uint32_t index = Hash(data.magic, data.shift, occupancy & data.mask);
    return data.attackTable[index];
}

Bitboard AttackTables::Bishop(Square square, Bitboard occupancy) const
{
    const MagicBitboard& data = m_bishop[square.Index()];
    uint32_t index = Hash(data.magic, data.shift, occupancy & data.mask);
    return data.attackTable[index];
}

Bitboard AttackTables::Queen(Square square, Bitboard occupancy) const
{
    return Rook(square, occupancy) | Bishop(square, occupancy);
}

constexpr Bitboard AttackTables::Hash(Bitboard magic, uint8_t shift, Bitboard occupancy) const
{
    return (occupancy * magic) >> shift;
}

const std::array<std::array<Bitboard, SquareCount>, Side::Count> AttackTables::InitPawnLookup()
{
    std::array<std::array<Bitboard, SquareCount>, Side::Count> lookup {};
    for (uint8_t rank = 0; rank < BoardSide; rank++) {
        for (uint8_t file = 0; file < BoardSide; file++) {
            Square square(file, rank);
            Bitboard bitboard = BB::Create(square.Index());
            
            // White
            lookup[Side::White][square.Index()] =
                (bitboard << 9 & ~BB::FileA) | // Up 1, Right 1
                (bitboard << 7 & ~BB::FileH);  // Up 1, Left 1
            
            // Black
            lookup[Side::Black][square.Index()] =
                (bitboard >> 7 & ~BB::FileA) | // Down 1, Right 1
                (bitboard >> 9 & ~BB::FileH);  // Down 1, Left 1
        }
    }
    return lookup;
}

const std::array<Bitboard, SquareCount> AttackTables::InitKnightLookup()
{
    std::array<Bitboard, SquareCount> lookup {};
    for (uint8_t rank = 0; rank < BoardSide; rank++) {
        for (uint8_t file = 0; file < BoardSide; file++) {
            Square square(file, rank);
            Bitboard bitboard = BB::Create(square.Index());
            
            lookup[square.Index()] =
                (bitboard << 17 & ~BB::FileA)  | // Up 2, Right 1
                (bitboard << 15 & ~BB::FileH)  | // Up 2, Left 1
                (bitboard << 10 & ~BB::FileAB) | // Up 1, Right 2
                (bitboard << 6 &  ~BB::FileGH) | // Up 1, Left 2
                (bitboard >> 6 &  ~BB::FileAB) | // Down 1, Right 2
                (bitboard >> 10 & ~BB::FileGH) | // Down 1, Left 2
                (bitboard >> 15 & ~BB::FileA)  | // Down 2, Right 1
                (bitboard >> 17 & ~BB::FileH);   // Down 2, Left 1
        }
    }
    return lookup;
}

const std::array<Bitboard, SquareCount> AttackTables::InitKingLookup()
{
    std::array<Bitboard, SquareCount> lookup {};
    for (uint8_t rank = 0; rank < BoardSide; rank++) {
        for (uint8_t file = 0; file < BoardSide; file++) {
            Square square(file, rank);
            Bitboard bitboard = BB::Create(square);
            
            lookup[square.Index()] = 
                (bitboard << 9 & ~BB::FileA) | // Up 1, Right 1
                (bitboard << 8)              | // Up 1
                (bitboard << 7 & ~BB::FileH) | // Up 1, Left 1
                (bitboard << 1 & ~BB::FileA) | // Right 1
                (bitboard >> 1 & ~BB::FileH) | // Left 1
                (bitboard >> 7 & ~BB::FileA) | // Down 1, Right 1
                (bitboard >> 8)              | // Down 1
                (bitboard >> 9 & ~BB::FileH);  // Down 1, Left 1
        }
    }
    return lookup;
}

const std::array<MagicBitboard, SquareCount> AttackTables::InitRookLookup()
{    
    std::array<MagicBitboard, SquareCount> lookup {};
    std::vector<Bitboard> numbers;
    numbers.reserve(SquareCount);

    for (uint8_t square = 0; square < SquareCount; square++) {
        lookup[square] = InitMagic(Type::Rook, square, RookMagicNumbers[square]);
        numbers.push_back(lookup[square].magic);
    }
    return lookup;
}

const std::array<MagicBitboard, SquareCount> AttackTables::InitBishopLookup()
{
    std::array<MagicBitboard, SquareCount> lookup {};
    std::vector<Bitboard> numbers;
    numbers.reserve(SquareCount);

    for (uint8_t square = 0; square < SquareCount; square++) {
        lookup[square] = InitMagic(Type::Bishop, square, BishopMagicNumbers[square]);
        numbers.push_back(lookup[square].magic);
    }
    return lookup;
}

Bitboard AttackTables::MagicMask(const uint8_t type, const Square square)
{
    Bitboard mask = 0;

    auto fill = [&](auto shift) {
        Bitboard dir = shift(BB::Create(square));
        while (shift(dir)) {
            mask |= dir;
            dir = shift(dir);
        }
    };

    if (type == Type::Rook) {
        fill(BB::N);
        fill(BB::S);
        fill(BB::E);
        fill(BB::W);
    } else if (type == Type::Bishop) {
        fill(BB::NE);
        fill(BB::NW);
        fill(BB::SE);
        fill(BB::SW);
    }
    return mask;
}

Bitboard AttackTables::MagicOccupancy(uint32_t index, Bitboard mask)
{
    // The index masks some bits to create the specific occupancy bitboard
    Bitboard occupancy = 0;
    while(mask) {
        Square square = BB::PopLSB(mask);
        if (index & 1) {
            occupancy |= BB::Create(square);
        }
        index >>= 1;
    }
    return occupancy;
}

Bitboard AttackTables::GenerateSlidingMoves(const uint8_t type, const Square square, const Bitboard occupancy)
{
    Bitboard attacks = 0;
    const Bitboard origin = BB::Create(square);

    auto trace = [&](auto shift) {
        Bitboard b = shift(origin);
        while (b) {
            attacks |= b;
            if (b & occupancy) break;
            b = shift(b);
        }
    };

    if (type == Type::Rook) {
        trace(BB::N);
        trace(BB::S);
        trace(BB::E);
        trace(BB::W);
    } else if (type == Type::Bishop) {
        trace(BB::NE);
        trace(BB::NW);
        trace(BB::SE);
        trace(BB::SW);
    }
    return attacks;
}

MagicBitboard AttackTables::InitMagic(uint8_t type, Square square, Bitboard magic)
{
    Bitboard mask = MagicMask(type, square);
    uint32_t bitCount = BB::Count(mask);
    uint32_t indexCount = 1u << bitCount; // Table size: 2^n
    uint8_t shift = 64 - bitCount;

    // Precompute all possible occupancies and their attacks
    std::vector<Bitboard> occupancy(indexCount, 0);
    std::vector<Bitboard> attacks(indexCount, 0);

    for (uint32_t i = 0; i < indexCount; i++) {
        occupancy[i] = MagicOccupancy(i, mask);
        attacks[i] = GenerateSlidingMoves(type, square, occupancy[i]);
    }

    std::vector<Bitboard> attackTable(indexCount);
    for (uint32_t i = 0; i < indexCount; i++) {
        uint32_t index = Hash(magic, shift, occupancy[i]);
        attackTable[index] = attacks[i];
    }

    return { std::move(attackTable), mask, magic, shift };
}

// Used as development utility to generate magic numbers
MagicBitboard AttackTables::BruteForceMagicNumbers(Bitboard mask, uint8_t shift, uint32_t indexCount,
    std::vector<Bitboard>& occupancy, std::vector<Bitboard>& attacks)
{
    // Brute-force find a magic number with zero collisions
    std::vector<Bitboard> attackTable(indexCount, 0);
    std::vector<uint32_t> used(indexCount, 0);

    static std::mt19937_64 gen(0xC0FFEE);

    uint32_t generation = 0;

    while (true) {
        generation++;

        // Sparse random magic candidate
        Bitboard magic = gen() & gen() & gen();

        // Reject candidates with poor bit distribution
        if (BB::Count((mask * magic) & Bitboard{0xFF00000000000000}) < 6) {
            continue;
        }
        
        bool collision = false;

        for (uint32_t i = 0; i < indexCount; i++) {
            uint32_t index = Hash(magic, shift, occupancy[i]);
            
            if (used[index] != generation) {
                used[index] = generation;
                attackTable[index] = attacks[i];
            } else if (attackTable[index] != attacks[i]) {
                collision = true;
                break;
            }
        }

        if (!collision) {
            return { std::move(attackTable), mask, magic, shift };
        }
    }
}

};
};