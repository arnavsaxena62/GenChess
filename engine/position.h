#pragma once
#include <cstdint>

#define u64 uint64_t

enum Color : int {
    WHITE,
    BLACK
};

class Position {
public:
    u64 whitePawn;
    u64 whiteKnight;
    u64 whiteRook;
    u64 whiteBishop;
    u64 whiteQueen;
    u64 whiteKing;

    u64 blackPawn;
    u64 blackKnight;
    u64 blackRook;
    u64 blackBishop;
    u64 blackQueen;
    u64 blackKing;

    u64 black() {
        return blackPawn | blackKnight | blackQueen |
               blackRook | blackBishop | blackKing;
    }

    u64 white() {
        return whitePawn | whiteKnight | whiteQueen |
               whiteRook | whiteBishop | whiteKing;
    }

    u64 occupied() {
        return black() | white();
    }
};