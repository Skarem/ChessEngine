#include "perft.hpp"

#include "fen.hpp"
#include "movegen.hpp"

#include <chrono>

namespace Chess 
{

uint64_t Perft::TestPosition(Position& position, int depth)
{
    if (depth == 0) return 1;

    std::vector<std::vector<Move>> buffers(depth);
    for (auto& buffer : buffers) buffer.reserve(256);

    auto visit = [&](auto&& self, int remaining, size_t ply) -> uint64_t {
        if (remaining == 0) return 1;

        auto& moves = buffers[ply];
        MoveGen::PseudoLegalMoves(position, moves);

        const uint8_t movingSide = position.sideToMove;
        uint64_t nodes = 0;

        for (const Move& move : moves) {
            UndoState undoState = MoveGen::MakeMove(position, move);

            if (!MoveGen::IsInCheck(position, movingSide))
                nodes += self(self, remaining - 1, ply + 1);
            
            MoveGen::UnmakeMove(position, move, undoState);
        }
        return nodes;
    };

    return visit(visit, depth, 0);
}

void Perft::TestBatchPositions()
{
    using Clock = std::chrono::steady_clock;
    std::println("\nValidate move generation\n");

    int section = 1;
    for (const auto& fenSection : m_perftTests) {
        std::println("Section #{}", section);
        for (const std::string& fenResult : fenSection) {
            PerftTest test = ParseTest(fenResult);
            
            const auto start = Clock::now();
            uint64_t count = TestPosition(test.position, test.depth);
            const auto end = Clock::now();

            const double sec = std::chrono::duration<double>(end - start).count();
            const double nps = count / sec;
            
            std::print("Depth: {}: ", test.depth);
            if (count != test.moveCount) {
                std::print("Perft generated {} moves, should be {} moves ", count, test.moveCount);
            } else {
                std::print("{:<12} ", test.moveCount);
            }
            std::println("({:.5f}s, {:>13.2f} nodes/s)", sec, nps);
        }
        section++;
        std::println();
    }
}

PerftTest Perft::ParseTest(std::string perftResult)
{
    std::vector<std::string> fields = FEN::SplitBySpaces(perftResult);

    if (fields.size() != 8) {
        throw std::invalid_argument("Perft result string must have exactly 8 fields");
    }

    std::string FEN;
    for (uint8_t i = 0; i < 6; i++) {
        FEN += (fields[i] + ' ');
    }

    Position position = FEN::LoadFEN(FEN);
    int depth = std::stoi(fields[6]);
    uint64_t count = std::stoull(fields[7]);

    return {
        position,
        depth,
        count
    };
}

};