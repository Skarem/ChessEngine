#include "fen.hpp"

#include <cassert>
#include <chrono>
#include <cstdio>
#include <print>
#include <sstream>

namespace Chess
{

Position FEN::LoadFEN(const std::string& FEN)
{
    std::vector<std::string> fields = SplitBySpaces(FEN);

    if (fields.size() != 6) {
        throw std::invalid_argument("FEN must have exactly 6 fields");
    }

    Position position {};

    ParsePiecePlacement(position,   fields[0]);
    ParseActiveColor(position,      fields[1]);
    ParseCastlingRights(position,   fields[2]);
    ParseEnPassant(position,        fields[3]);
    ParseHalfmoveClock(position,    fields[4]);
    ParseFullmoveNumber(position,   fields[5]);

    for (uint8_t color = 0; color < Side::Count; color++) {
        position.board.occupancy[color] = BB::SideOccupancy(position.board.pieces[color]);
        if (BB::Count(position.board.pieces[color][Type::King]) != 1)
            throw std::invalid_argument("FEN must contain exactly one king per color");
    }
    return position;
}

std::vector<std::string> FEN::SplitBySpaces(const std::string& str)
{
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string word;
    while (iss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

void FEN::ParsePiecePlacement(Position& position, const std::string& field)
{
    int file = 0;
    int rank = 7;

    for (const char c : field) {
        if (rank < 0 || file > 8) throw std::invalid_argument("FEN piece placement exceeds board bounds");

        if (c >= '1' && c <= '8') {
            file += c - '0';
            if (file > 8) throw std::invalid_argument("FEN rank contains too many squares");
        }
        else if (c == '/') {
            if (file != 8 || rank == 0) throw std::invalid_argument("FEN rank length or count invalid");
            rank--;
            file = 0;
        }
        else {
            if (file >= 8 || rank < 0) throw std::invalid_argument("FEN piece outside board");
            Square sq(file, rank);

            switch (c) {
                case 'P': BB::Set(position.board.pieces[Side::White][Type::Pawn],   sq); file++; break;
                case 'N': BB::Set(position.board.pieces[Side::White][Type::Knight], sq); file++; break;
                case 'B': BB::Set(position.board.pieces[Side::White][Type::Bishop], sq); file++; break;
                case 'R': BB::Set(position.board.pieces[Side::White][Type::Rook],   sq); file++; break;
                case 'Q': BB::Set(position.board.pieces[Side::White][Type::Queen],  sq); file++; break;
                case 'K': BB::Set(position.board.pieces[Side::White][Type::King],   sq); file++; break;

                case 'p': BB::Set(position.board.pieces[Side::Black][Type::Pawn],   sq); file++; break;
                case 'n': BB::Set(position.board.pieces[Side::Black][Type::Knight], sq); file++; break;
                case 'b': BB::Set(position.board.pieces[Side::Black][Type::Bishop], sq); file++; break;
                case 'r': BB::Set(position.board.pieces[Side::Black][Type::Rook],   sq); file++; break;
                case 'q': BB::Set(position.board.pieces[Side::Black][Type::Queen],  sq); file++; break;
                case 'k': BB::Set(position.board.pieces[Side::Black][Type::King],   sq); file++; break;
                default: throw std::invalid_argument("Invalid FEN piece character");
            }
        }
    }
    if (rank != 0 || file != 8) throw std::invalid_argument("FEN piece placement must contain eight complete ranks");
}

void FEN::ParseActiveColor(Position& position, const std::string& field)
{
    for (const char c : field) {
        if (c == 'w') {
            position.sideToMove = Side::White;
        } else if (c == 'b') {
            position.sideToMove = Side::Black;
        } else {
            std::println(stderr, "FEN: Invalid color: {}", c);
        }
    }
}

void FEN::ParseCastlingRights(Position& position, const std::string& field)
{
    for (const char c : field) {
        if      (c == 'K') Castling::Set(position.castlingRights, Castling::WhiteKingside);
        else if (c == 'Q') Castling::Set(position.castlingRights, Castling::WhiteQueenside);
        else if (c == 'k') Castling::Set(position.castlingRights, Castling::BlackKingside);
        else if (c == 'q') Castling::Set(position.castlingRights, Castling::BlackQueenside);
        else if (c == '-') continue;
        else std::println(stderr, "FEN: Invalid character in castling rights: {}", c);
    }
}

void FEN::ParseEnPassant(Position& position, const std::string& field)
{
    int enPassantFile = 0;
    int enPassantRank = 0;
    for (const char c : field) {
        if (c == '-') {
            position.enPassantTarget = NoSquare;
        } else if (isalpha(c)) {
            enPassantFile = c - 'a';
            if (enPassantFile < 0 || enPassantFile >= BoardSide) {
                std::println(stderr, "FEN: Invalid en passant file target: {}", enPassantFile);
            }
        } else if (isdigit(c)) {
            enPassantRank = c - '0' - 1;
            if (enPassantRank < 0 || enPassantRank >= BoardSide) {
                std::println(stderr, "FEN: Invalid en passant rank target: {}", enPassantRank);
            }
            position.enPassantTarget = Square(enPassantFile, enPassantRank);
        }
    }
}

void FEN::ParseHalfmoveClock(Position& position, const std::string& field)
{
    for (const char c : field) {
        position.halfmoveClock = position.halfmoveClock * 10 + (c - '0');
    }
}

void FEN::ParseFullmoveNumber(Position& position, const std::string& field)
{
    position.fullmoveNum = 0;
    for (const char c : field) {
        position.fullmoveNum = position.fullmoveNum * 10 + (c - '0');
    }
}

bool FEN::EqualPositions(Position prev, Position curr)
{
    bool equal = true;

    for (uint8_t color = 0; color < Side::Count; color++) {
        for (uint8_t type = 0; type < Type::Count; type++) {
            if (prev.board.pieces[color][type] != curr.board.pieces[color][type]) equal = false;
        }
        if (prev.board.occupancy[color] != curr.board.occupancy[color]) equal = false;
    }

    if (prev.castlingRights != curr.castlingRights || prev.enPassantTarget.Index() != curr.enPassantTarget.Index() ||
        prev.fullmoveNum    != curr.fullmoveNum    || prev.halfmoveClock   != curr.halfmoveClock  ||
        prev.sideToMove     != curr.sideToMove) equal = false;
    
    return equal;
}

};