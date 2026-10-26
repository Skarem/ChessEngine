#include "movegen.hpp"

#include "lookup.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <print>
#include <utility>

namespace Chess
{

UndoState MoveGen::MakeMove(Position& position, const Move& move)
{
    UndoState undoState;
    undoState.prevCastlingRights = position.castlingRights;
    undoState.prevEnPassantTarget = position.enPassantTarget;
    undoState.prevHalfmoveClock = position.halfmoveClock;

    assert(move.from.Index() < SquareCount && move.to.Index() < SquareCount && "Invalid square locations");

    uint8_t ownSide = position.sideToMove;
    uint8_t otherSide = Side::Other(ownSide);

    // Get moving piece type
    uint8_t type = GetType(position.board, move.from, ownSide);
    assert(type < Type::Count && "No moving piece on move.from");

    // Get captured piece type
    uint8_t capturedType = GetType(position.board, move.to, otherSide);
    undoState.capturedType = capturedType;
    assert(capturedType != Type::King && "Cannot capture the king");
    bool capture = capturedType != Type::None;
    
    // Remove the moving piece and normally captured piece
    BB::Clear(position.board.pieces[ownSide][type], move.from);
    if (capture) BB::Clear(position.board.pieces[otherSide][capturedType], move.to);

    // En passant captures the pawn behind the destination
    if (move.type == MoveType::EnPassant) {
        assert(type == Type::Pawn && "Piece capturing en passant must be a pawn");

        const int WHITE_CAPTURE = -8;
        const int BLACK_CAPTURE =  8;
        Square capturedSquare = static_cast<Square>(int(move.to.Index()) + (ownSide == Side::White ? WHITE_CAPTURE : BLACK_CAPTURE));
        Bitboard& otherPawns = position.board.pieces[otherSide][Type::Pawn];
        assert(BB::Test(otherPawns, capturedSquare) && "Piece being captured en passant must be a pawn");

        BB::Clear(otherPawns, capturedSquare);
        undoState.capturedType = Type::Pawn;
        capture = true;
    }

    // Place the moving piece, or the promoted piece
    if (move.type == MoveType::Promotion) {
        assert(type == Type::Pawn && "Piece promoted must be a pawn");
        assert(move.promotion >= Type::Knight && move.promotion <= Type::Queen && "MakeMove: Invalid piece promotion type");
        BB::Set(position.board.pieces[ownSide][move.promotion], move.to);
    } else {
        BB::Set(position.board.pieces[ownSide][type], move.to);
    }

    // Castling also requires moving the rook
    if (move.type == MoveType::Castling) {
        assert(type == Type::King && "Castling must be king");

        const uint8_t FILE_FROM_KINGSIDE  = 7;
        const uint8_t FILE_FROM_QUEENSIDE = 0;
        const uint8_t FILE_TO_KINGSIDE    = 5;
        const uint8_t FILE_TO_QUEENSIDE   = 3;

        bool kingside = move.to.File() > move.from.File();
        Square rookFrom = Square(kingside ? FILE_FROM_KINGSIDE : FILE_FROM_QUEENSIDE, move.from.Rank());
        Square rookTo = Square(kingside ? FILE_TO_KINGSIDE : FILE_TO_QUEENSIDE, move.from.Rank());

        assert(BB::Test(position.board.pieces[ownSide][Type::Rook], rookFrom) && "Can't castle without rook");

        BB::Clear(position.board.pieces[ownSide][Type::Rook], rookFrom);
        BB::Set(position.board.pieces[ownSide][Type::Rook], rookTo);
    }

    // Moving the king permanently removes its castling rights
    if (type == Type::King) {
        uint8_t mask = ownSide == Side::White ? Castling::WhiteRights : Castling::BlackRights;
        Castling::Clear(position.castlingRights, mask);
    }

    // Moving or capturing an original rook removes the corresponding right
    auto clearRookRight = [&](Square square, uint8_t side) {
        if (side == Side::White) {
            if (square.Index() == Square(0, 0).Index()) Castling::Clear(position.castlingRights, Castling::WhiteQueenside);
            if (square.Index() == Square(7, 0).Index()) Castling::Clear(position.castlingRights, Castling::WhiteKingside);
        } else {
            if (square.Index() == Square(0, 7).Index()) Castling::Clear(position.castlingRights, Castling::BlackQueenside);
            if (square.Index() == Square(7, 7).Index()) Castling::Clear(position.castlingRights, Castling::BlackKingside);
        }
    };
    if (type == Type::Rook) clearRookRight(move.from, ownSide);
    if (capturedType == Type::Rook) clearRookRight(move.to, otherSide);

    // En passant target is available only after a double pawn push
    position.enPassantTarget = NoSquare;

    const int MOVE_DISANCE = std::abs(int(move.to.Index()) - int(move.from.Index()));
    const int DOUBLE_PAWN_PUSH_DISTANCE = 16;
    if (type == Type::Pawn && MOVE_DISANCE == DOUBLE_PAWN_PUSH_DISTANCE)
        position.enPassantTarget = static_cast<Square>((int(move.from.Index()) + int(move.to.Index())) / 2);

    // Update clock
    
    // Resest if pawn move or capture
    position.halfmoveClock = (type == Type::Pawn || capture) ? 0 : position.halfmoveClock + 1;

    if (ownSide == Side::Black) position.fullmoveNum++;

    // Update occupancies
    position.board.occupancy[ownSide]   = BB::SideOccupancy(position.board.pieces[ownSide]);
    position.board.occupancy[otherSide] = BB::SideOccupancy(position.board.pieces[otherSide]);
    
    // Switch sides
    position.sideToMove = otherSide;

    return undoState;
}

void MoveGen::UnmakeMove(Position& position, const Move& move, const UndoState& undoState)
{
    assert(move.from.Index() < SquareCount && move.to.Index() < SquareCount && "Invalid square locations");

    uint8_t currSide = position.sideToMove;
    uint8_t prevSide = Side::Other(currSide);

    Square currSquare = move.to;
    Square prevSquare = move.from;

    // Get moving piece type
    uint8_t type = GetType(position.board, currSquare, prevSide);
    assert(type < Type::Count && "No moving piece on current square");

    // Get captured piece type
    uint8_t capturedType = undoState.capturedType;
    bool capture = capturedType != Type::None;

    // Reverse current piece move
    BB::Clear(position.board.pieces[prevSide][type], currSquare);

    // Place back piece
    if (move.type == MoveType::Promotion) {
        BB::Set(position.board.pieces[prevSide][Type::Pawn], prevSquare);
    } else {
        BB::Set(position.board.pieces[prevSide][type], prevSquare);
    }
    
    // Place back captured piece
    if (capture) {
        // En passant places back the pawn behind the destination
        if (move.type == MoveType::EnPassant) {
            assert(capturedType == Type::Pawn && "Piece captured en passant must be a pawn");

            const int WHITE_CAPTURE = -8;
            const int BLACK_CAPTURE =  8;
            Square pawnSquare = static_cast<Square>((int)currSquare.Index() + (prevSide == Side::White ? WHITE_CAPTURE : BLACK_CAPTURE));
            BB::Set(position.board.pieces[currSide][capturedType], pawnSquare);
        } else {
            BB::Set(position.board.pieces[currSide][capturedType], currSquare);
        }
    }

    // Castling also requires moving the rook
    if (move.type == MoveType::Castling) {
        assert(type == Type::King && "Castling must be king");

        const uint8_t FILE_FROM_KINGSIDE  = 7;
        const uint8_t FILE_FROM_QUEENSIDE = 0;
        const uint8_t FILE_TO_KINGSIDE    = 5;
        const uint8_t FILE_TO_QUEENSIDE   = 3;

        bool kingside = currSquare.File() > prevSquare.File();
        Square rookFrom = Square(kingside ? FILE_FROM_KINGSIDE : FILE_FROM_QUEENSIDE, move.from.Rank());
        Square rookTo = Square(kingside ? FILE_TO_KINGSIDE : FILE_TO_QUEENSIDE, move.from.Rank());
        
        BB::Clear(position.board.pieces[prevSide][Type::Rook], rookTo);
        BB::Set(position.board.pieces[prevSide][Type::Rook], rookFrom);
    }

    // Revert castling rights
    position.castlingRights = undoState.prevCastlingRights;

    // Revert en passant target
    position.enPassantTarget = undoState.prevEnPassantTarget;

    // Revert clock
    position.halfmoveClock = undoState.prevHalfmoveClock;

    if (currSide == Side::White) position.fullmoveNum--;

    // Update occupancies
    position.board.occupancy[currSide] = BB::SideOccupancy(position.board.pieces[currSide]);
    position.board.occupancy[prevSide] = BB::SideOccupancy(position.board.pieces[prevSide]);

    // Switch sides
    position.sideToMove = prevSide;
}

std::vector<Move> MoveGen::PseudoLegalMoves(const Position& position)
{
    std::vector<Move> moves;
    moves.reserve(256);
    PseudoLegalMoves(position, moves);
    return moves;  
}

void MoveGen::PseudoLegalMoves(const Position& position, std::vector<Move>& moves)
{
    moves.clear();
    
    uint8_t side = position.sideToMove;
    const auto& pieces = position.board.pieces[side];
    const auto& ownOccupancy = position.board.occupancy[side];
    const auto& otherOccupancy = position.board.occupancy[Side::Other(side)];

    Bitboard enemyKing = position.board.pieces[Side::Other(side)][Type::King];

    PawnAttacks  (moves, pieces[Type::Pawn],   ownOccupancy, otherOccupancy, position.enPassantTarget, side);
    KnightAttacks(moves, pieces[Type::Knight], ownOccupancy);
    BishopAttacks(moves, pieces[Type::Bishop], ownOccupancy, otherOccupancy);
    RookAttacks  (moves, pieces[Type::Rook],   ownOccupancy, otherOccupancy);
    QueenAttacks (moves, pieces[Type::Queen],  ownOccupancy, otherOccupancy);
    KingAttacks  (moves, pieces[Type::King],   ownOccupancy);
    CastlingMoves(moves, position, side);

    // King can't be captured
    std::erase_if(moves, [&](const Move& move) {
        return BB::Test(enemyKing, move.to);
    });
}

void MoveGen::LegalMoves(Position& position, std::vector<Move>& moves)
{
    PseudoLegalMoves(position, moves);

    const uint8_t movingSide = position.sideToMove;
    size_t legalCount = 0;

    for (size_t i = 0; i < moves.size(); i++) {
        Move move = moves[i];

        UndoState undo = MakeMove(position, move);
        bool legal = !IsInCheck(position, movingSide);
        UnmakeMove(position, move, undo);

        if (legal) moves[legalCount++] = move;
    }
    moves.erase(moves.begin() + legalCount, moves.end());
}

std::vector<Move> MoveGen::LegalMoves(const Position& position)
{
    Position copy = position;
    std::vector<Move> moves;
    moves.reserve(256);
    LegalMoves(copy, moves);
    return moves;
}

void MoveGen::PawnAttacks(std::vector<Move>& moves, Bitboard pawns, Bitboard occupancyOwn, Bitboard occupancyOther, Square enPassantTarget, uint8_t side)
{
    Bitboard emptySquares = ~(occupancyOwn | occupancyOther);
    bool white = side == Side::White;
    int direction = white ? BoardSide : -BoardSide;

    // Single and double pawn pushes
    Bitboard singlePushes = white ? BB::N(pawns) : BB::S(pawns);
    singlePushes &= emptySquares;

    Bitboard doublePushes = white ? BB::N(singlePushes & BB::Rank3) : BB::S(singlePushes & BB::Rank6);
    doublePushes &= emptySquares;

    while (singlePushes) {
        Square to = BB::PopLSB(singlePushes);
        AddPawnMove(moves, to.Index() - direction, to, side);
    }
    while (doublePushes) {
        Square to = BB::PopLSB(doublePushes);
        Square from = (to.Index() - 2 * direction);
        moves.push_back(Move{ from, to });
    }
    
    // Captures
    Bitboard targets = occupancyOther;
    if (enPassantTarget.Index() != NoSquare) {
        targets |= BB::Create(enPassantTarget);
    }
    while(pawns) {
        Square from = BB::PopLSB(pawns);
        ;
        Bitboard attacks = Lookup::Get().Pawn(side, from) & targets;
        while (attacks) {
            Square to = BB::PopLSB(attacks);
            if (to.Index() == enPassantTarget.Index()) {
                moves.push_back({ from, to, MoveType::EnPassant });
            } else {
                AddPawnMove(moves, from, to, side);
            }
        }
    }
}

void MoveGen::KnightAttacks(std::vector<Move>& moves, Bitboard knights, Bitboard occupancyOwn)
{
    Bitboard freeSquares = ~occupancyOwn;
    while (knights) {
        Square from = BB::PopLSB(knights);
        Bitboard attacks = Lookup::Get().Knight(from) & freeSquares;

        while (attacks) {
            Square to = BB::PopLSB(attacks);
            moves.push_back({ from, to });
        }
    }
}

void MoveGen::BishopAttacks(std::vector<Move>& moves, Bitboard bishops, Bitboard occupancyOwn, Bitboard occupancyOther)
{
    Bitboard occupancyAll = occupancyOwn | occupancyOther;
    while (bishops) {
        Square from = BB::PopLSB(bishops);
        Bitboard attacks = Lookup::Get().Bishop(from, occupancyAll);
        attacks &= ~occupancyOwn;

        while (attacks) {
            Square to = BB::PopLSB(attacks);
            moves.push_back({ from, to });
        }
    }
}

void MoveGen::RookAttacks(std::vector<Move>& moves, Bitboard rooks, Bitboard occupancyOwn, Bitboard occupancyOther)
{
    Bitboard occupancyAll = occupancyOwn | occupancyOther;
    while (rooks) {
        Square from = BB::PopLSB(rooks);
        Bitboard attacks = Lookup::Get().Rook(from, occupancyAll);
        attacks &= ~occupancyOwn;

        while (attacks) {
            Square to = BB::PopLSB(attacks);
            moves.push_back({ from, to });
        }
    }
}

void MoveGen::QueenAttacks(std::vector<Move>& moves, Bitboard queens, Bitboard occupancyOwn, Bitboard occupancyOther)
{
    Bitboard occupancyAll = occupancyOwn | occupancyOther;
    while (queens) {
        Square from = BB::PopLSB(queens);
        Bitboard attacks = Lookup::Get().Queen(from, occupancyAll);
        attacks &= ~occupancyOwn;

        while (attacks) {
            Square to = BB::PopLSB(attacks);
            moves.push_back({ from, to });
        }
    }
}

void MoveGen::KingAttacks(std::vector<Move>& moves, Bitboard king, Bitboard occupancyOwn)
{
    Bitboard freeSquares = ~occupancyOwn;
    if (!king) return;
    Square from = BB::PopLSB(king);
    Bitboard attacks = Lookup::Get().King(from) & freeSquares;

    while (attacks) {
        Square to = BB::PopLSB(attacks);
        moves.push_back({ from, to });
    }
}

void MoveGen::AddPawnMove(std::vector<Move>& moves, Square from, Square to, uint8_t side)
{
    Bitboard promotionRank = (side == Side::White) ? BB::Rank8 : BB::Rank1;
    if (BB::Create(to) & promotionRank) {
        moves.push_back({ from, to, MoveType::Promotion, Type::Knight });
        moves.push_back({ from, to, MoveType::Promotion, Type::Bishop });
        moves.push_back({ from, to, MoveType::Promotion, Type::Rook });
        moves.push_back({ from, to, MoveType::Promotion, Type::Queen });
    } else {
        moves.push_back({ from, to });
    }
}

uint8_t MoveGen::GetType(const Board& board, Square square, uint8_t side)
{
    for (uint8_t type = 0; type < Type::Count; type++) {
        if (BB::Test(board.pieces[side][type], square)) {
            return type;
        }
    }
    return Type::None;
}

bool MoveGen::IsSquareAttacked(const Position& position, Square square, uint8_t byColor)
{
    Bitboard occupied = position.board.occupancy[Side::White] | position.board.occupancy[Side::Black];
    return IsSquareAttacked(position, square, byColor, occupied);
}

bool MoveGen::IsSquareAttacked(const Position& position, Square square, uint8_t byColor, Bitboard occupancy)
{
    const auto& pieces = position.board.pieces[byColor];

    // Pawn attacks must be checked in the opposite direction.
    if (Lookup::Get().Pawn(Side::Other(byColor), square) & pieces[Type::Pawn])
        return true;

    if (Lookup::Get().Knight(square) & pieces[Type::Knight])
        return true;

    if (Lookup::Get().King(square) & pieces[Type::King])
        return true;

    if (Lookup::Get().Bishop(square, occupancy) & (pieces[Type::Bishop] | pieces[Type::Queen]))
        return true;

    if (Lookup::Get().Rook(square, occupancy) & (pieces[Type::Rook] | pieces[Type::Queen]))
        return true;

    return false;
}

bool MoveGen::IsInCheck(const Position& position, uint8_t side)
{
    Bitboard king = position.board.pieces[side][Type::King];
    assert(BB::Count(king) == 1 && "Missing or duplicated king");
    return IsSquareAttacked(position, BB::LSB(king), Side::Other(side));
}

void MoveGen::CastlingMoves(std::vector<Move>& moves, const Position& position, uint8_t side)
{   
    const uint8_t rightKingside  = (side == Side::White) ? Castling::WhiteKingside : Castling::BlackKingside;
    const uint8_t rightQueenside = (side == Side::White) ? Castling::WhiteQueenside : Castling::BlackQueenside;

    bool canCastleKingside  = Castling::HasRights(position.castlingRights, rightKingside);
    bool canCastleQueenside = Castling::HasRights(position.castlingRights, rightQueenside);

    // Check castling rights
    if (!canCastleKingside && !canCastleQueenside) return;

    const uint8_t rank = side == Side::White ? 0 : 7;
    const uint8_t enemy = Side::Other(side);
   
    const Square kingFrom = Square(4, rank);
    
    const Square b(1, rank);
    const Square c(2, rank);
    const Square d(3, rank);
    const Square f(5, rank);
    const Square g(6, rank);

    Bitboard occupied = position.board.occupancy[Side::White] | position.board.occupancy[Side::Black];
    Bitboard emptyKingside  = BB::Create(f) | BB::Create(g);
    Bitboard emptyQueenside = BB::Create(b) | BB::Create(c) | BB::Create(d);

    canCastleKingside  &= !(occupied & emptyKingside);
    canCastleQueenside &= !(occupied & emptyQueenside);

    // Check if path is clear
    if (!canCastleKingside && !canCastleQueenside) return;

    // King is attacked
    if (IsSquareAttacked(position, kingFrom, enemy)) return;
    
    // Remove the king from occupancy to reveal attacks through its original square.
    Bitboard occupiedWithoutKing = occupied;
    BB::Clear(occupiedWithoutKing, kingFrom);

    // The destination square is checked later by MakeMove() + IsInCheck().
    if (canCastleKingside && !IsSquareAttacked(position, f, enemy, occupiedWithoutKing))
        moves.push_back({ kingFrom, g, MoveType::Castling });

    if (canCastleQueenside && !IsSquareAttacked(position, d, enemy, occupiedWithoutKing))
        moves.push_back({ kingFrom, c, MoveType::Castling });
}

};