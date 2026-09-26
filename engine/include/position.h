#pragma once
#include "utils.h"
#include <cstdint>
#include <iostream>
using namespace std;

#define u64 uint64_t

enum Square : uint64_t {
    A1 = 1ULL << 0,
    B1 = 1ULL << 1,
    C1 = 1ULL << 2,
    D1 = 1ULL << 3,
    E1 = 1ULL << 4,
    F1 = 1ULL << 5,
    G1 = 1ULL << 6,
    H1 = 1ULL << 7,

    A2 = 1ULL << 8,
    B2 = 1ULL << 9,
    C2 = 1ULL << 10,
    D2 = 1ULL << 11,
    E2 = 1ULL << 12,
    F2 = 1ULL << 13,
    G2 = 1ULL << 14,
    H2 = 1ULL << 15,
};



class Position {
  public:
    u64 whitePawn = 0;
    u64 whiteKnight = 0;
    u64 whiteRook = 0;
    u64 whiteBishop = 0;
    u64 whiteQueen = 0;
    u64 whiteKing = 0;

    u64 blackPawn = 0;
    u64 blackKnight = 0;
    u64 blackRook = 0;
    u64 blackBishop = 0;
    u64 blackQueen = 0;
    u64 blackKing = 0;

    Position() {
        whitePawn = 0;
        whiteKnight = 0;
        whiteRook = 0;
        whiteBishop = 0;
        whiteQueen = 0;
        whiteKing = 0;

        blackPawn = 0;
        blackKnight = 0;
        blackRook = 0;
        blackBishop = 0;
        blackQueen = 0;
        blackKing = 0;
    }

    Position(bool starting) {
        whitePawn = 0;
        whiteKnight = 0;
        whiteRook = 0;
        whiteBishop = 0;
        whiteQueen = 0;
        whiteKing = 0;

        blackPawn = 0;
        blackKnight = 0;
        blackRook = 0;
        blackBishop = 0;
        blackQueen = 0;
        blackKing = 0;

        if (!starting)
            return;

        // White
        whitePawn = 0x000000000000FF00ULL;
        whiteRook = 0x0000000000000081ULL;
        whiteKnight = 0x0000000000000042ULL;
        whiteBishop = 0x0000000000000024ULL;
        whiteQueen = 0x0000000000000008ULL;
        whiteKing = 0x0000000000000010ULL;

        // Black
        blackPawn = 0x00FF000000000000ULL;
        blackRook = 0x8100000000000000ULL;
        blackKnight = 0x4200000000000000ULL;
        blackBishop = 0x2400000000000000ULL;
        blackQueen = 0x0800000000000000ULL;
        blackKing = 0x1000000000000000ULL;
    }

    Position(Position &position) {
        whitePawn = position.whitePawn;
        whiteKnight = position.whiteKnight;
        whiteRook = position.whiteRook;
        whiteBishop = position.whiteBishop;
        whiteQueen = position.whiteQueen;
        whiteKing = position.whiteKing;
        blackPawn = position.blackPawn;
        blackKnight = position.blackKnight;
        blackRook = position.blackRook;
        blackBishop = position.blackBishop;
        blackQueen = position.blackQueen;
        blackKing = position.blackKing;
    }

    u64 black() {
        return blackPawn | blackKnight | blackQueen | blackRook | blackBishop | blackKing;
    }

    u64 white() {
        return whitePawn | whiteKnight | whiteQueen | whiteRook | whiteBishop | whiteKing;
    }

    u64 occupied() { return black() | white(); }

    u64 own(Color c) {
        if (c == WHITE)
            return white();

        return black();
    }

    void makeMove(Move move) {
        u64 pieceBB = InttoBB(move.from);
        u64 targetBB = InttoBB(move.to);

        Color movingColor = (pieceBB & white()) ? WHITE : BLACK;

        if (movingColor == WHITE) {
            u64 *whiteBBs[6] = {&whitePawn,   &whiteKnight, &whiteRook,
                                &whiteBishop, &whiteQueen,  &whiteKing};
            for (auto bb : whiteBBs) {
                if (*bb & pieceBB) {
                    *bb &= ~pieceBB; // remove from source
                    *bb |= targetBB; // place on target
                    break;
                }
            }

            u64 capturedBB = targetBB & black();
            blackPawn &= ~capturedBB;
            blackKnight &= ~capturedBB;
            blackRook &= ~capturedBB;
            blackBishop &= ~capturedBB;
            blackQueen &= ~capturedBB;
            blackKing &= ~capturedBB;
        } else {
            u64 *blackBBs[6] = {&blackPawn,   &blackKnight, &blackRook,
                                &blackBishop, &blackQueen,  &blackKing};
            for (auto bb : blackBBs) {
                if (*bb & pieceBB) {
                    *bb &= ~pieceBB;
                    *bb |= targetBB;
                    break;
                }
            }

            u64 capturedBB = targetBB & white();
            whitePawn &= ~capturedBB;
            whiteKnight &= ~capturedBB;
            whiteRook &= ~capturedBB;
            whiteBishop &= ~capturedBB;
            whiteQueen &= ~capturedBB;
            whiteKing &= ~capturedBB;
        }
    }

    void display() {
        for (int rank = 7; rank >= 0; rank--) {
            cout << rank + 1 << " ";

            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                u64 bit = 1ULL << square;

                char piece = '.';

                if (whitePawn & bit)
                    piece = 'P';
                else if (whiteKnight & bit)
                    piece = 'N';
                else if (whiteBishop & bit)
                    piece = 'B';
                else if (whiteRook & bit)
                    piece = 'R';
                else if (whiteQueen & bit)
                    piece = 'Q';
                else if (whiteKing & bit)
                    piece = 'K';

                else if (blackPawn & bit)
                    piece = 'p';
                else if (blackKnight & bit)
                    piece = 'n';
                else if (blackBishop & bit)
                    piece = 'b';
                else if (blackRook & bit)
                    piece = 'r';
                else if (blackQueen & bit)
                    piece = 'q';
                else if (blackKing & bit)
                    piece = 'k';

                cout << piece << " ";
            }

            cout << '\n';
        }

        cout << "  a b c d e f g h\n";
    }
};