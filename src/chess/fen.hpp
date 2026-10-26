#pragma once

#include "position.hpp"
#include "movegen.hpp"

namespace Chess 
{

class FEN
{
public:
    static Position LoadFEN(const std::string& FEN);
    static std::vector<std::string> SplitBySpaces(const std::string& str);
    
private:
    static void ParsePiecePlacement(Position& position, const std::string& field);
    static void ParseActiveColor   (Position& position, const std::string& field);
    static void ParseCastlingRights(Position& position, const std::string& field);
    static void ParseEnPassant     (Position& position, const std::string& field);
    static void ParseHalfmoveClock (Position& position, const std::string& field);
    static void ParseFullmoveNumber(Position& position, const std::string& field);

    static bool EqualPositions(Position prev, Position curr);

    FEN() = delete;
};

};