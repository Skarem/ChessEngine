#include "engine.hpp"

namespace Chess
{

Engine::Engine()
{
    InitTables();

    for (auto& moves : m_moveBuffers) moves.reserve(256);
}

int Engine::Eval(const Position& position)
{
    int midGame[Side::Count] = {};
    int endGame[Side::Count] = {};
    int gamePhase = 0;

    for (uint8_t side = 0; side < Side::Count; side++) {
        for (uint8_t type = 0; type < Type::Count; type++) {
            Bitboard pieces = position.board.pieces[side][type];

            while (pieces) {
                Square square = BB::PopLSB(pieces);
                midGame[side] += MidGameTable[side][type][square.Index()];
                endGame[side] += EndGameTable[side][type][square.Index()];
                gamePhase += GamePhaseInc[type];
            }
        }
    }

    uint8_t side  = position.sideToMove;
    uint8_t other = Side::Other(side);

    // Tappered eval
    int midGameScore = midGame[side] - midGame[other];
    int endGameScore = endGame[side] - endGame[other];

    int midGamePhase = std::min(gamePhase, MaxPhaseWeight);
    int endGamePhase = MaxPhaseWeight - midGamePhase;

    return (midGameScore * midGamePhase + endGameScore * endGamePhase) / MaxPhaseWeight;
}

std::optional<Move> Engine::FindBestMove(const Position& position, int depth)
{
    if (depth < 1) return std::nullopt;

    Position pos = position;
    auto moves = MoveGen::LegalMoves(pos);

    std::optional<Move> bestMove;
    int bestScore = -Infinity;

    for (const Move& move : moves) {
        UndoState undo = MoveGen::MakeMove(pos, move);
        int score = -NegaMax(pos, depth - 1, 1);
        MoveGen::UnmakeMove(pos, move, undo);

        if (!bestMove || score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    return bestMove;
}

int Engine::NegaMax(Position& pos, int depth, int ply)
{
    auto& moves = m_moveBuffers[ply];
    MoveGen::LegalMoves(pos, moves);

    if (moves.empty()) {
        return MoveGen::IsInCheck(pos, pos.sideToMove) ? -MateScore + ply : 0;
    }

    if (pos.halfmoveClock >= 100) return 0;

    if (depth == 0) return Eval(pos);

    int bestScore = -Infinity;

    for (const Move& move : moves) {
        UndoState undo = MoveGen::MakeMove(pos, move);
        int score = -NegaMax(pos, depth - 1, ply + 1);
        MoveGen::UnmakeMove(pos, move, undo);

        bestScore = std::max(bestScore, score);
    }
    return bestScore;
}

void Engine::InitTables()
{
    for (uint8_t side = 0; side < Side::Count; side++) {
        for (uint8_t type = 0; type < Type::Count; type++) {
            for (uint8_t square = 0; square < SquareCount; square++) {
                // Flip board depending on side
                int index = side == Side::White ? (square ^ 56) : square;
                MidGameTable[side][type][square] = MidGameValue[type] + MidGamePestoTable[type][index];
                EndGameTable[side][type][square] = EndGameValue[type] + EndGamePestoTable[type][index];
            }
        }
    }
}

};