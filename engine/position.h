#pragma once
#include <cstdint>
#include <iostream>
using namespace std;

#define u64 uint64_t

enum Square : uint64_t
{
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

enum Color : int
{
    WHITE,
    BLACK
};

class Position
{
  public:
    u64 whitePawn=0;
    u64 whiteKnight=0;
    u64 whiteRook=0;
    u64 whiteBishop=0;
    u64 whiteQueen=0;
    u64 whiteKing=0;

    u64 blackPawn=0;
    u64 blackKnight=0;
    u64 blackRook=0;
    u64 blackBishop=0;
    u64 blackQueen=0;
    u64 blackKing=0;

    u64 black()
    {
        return blackPawn | blackKnight | blackQueen | blackRook | blackBishop | blackKing;
    }

    u64 white()
    {
        return whitePawn | whiteKnight | whiteQueen | whiteRook | whiteBishop | whiteKing;
    }

    u64 occupied()
    {
        return black() | white();
    }

    void display()
    {
        for (int rank = 7; rank >= 0; rank--)
        {
            cout << rank + 1 << " ";

            for (int file = 0; file < 8; file++)
            {
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