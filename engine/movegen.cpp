#include "movegen.h"
#include <vector>
using namespace std;
#define u64 uint64_t

void MoveGen::Pawn(Position &position, Color color, std::vector<Move> &moves)
{
    if (color == WHITE)
    {
        for (int i = A2; i <= 64; i << 2)
        {
            u64 freeSquares = ~position.occupied();
        }
    }
    else if (color == BLACK)
    {
    }
}

void MoveGen::Knight(Position &position, Color color, std::vector<Move> &moves)
{
    u64 freeSquares = ~position.occupied();
    u64 moves[8] = {8+8+1, 8+8-1, -8-8+1, -8-8-1, 1+1+8, 1+1-8, -1-1-8, -1-1+8};
    
}

void MoveGen::King(Position &position, Color color, std::vector<Move> &moves)
{
}

void MoveGen::Bishop(Position &position, Color color, std::vector<Move> &moves)
{
}

void MoveGen::Queen(Position &position, Color color, std::vector<Move> &moves)
{
}

void MoveGen::Rook(Position &position, Color color, std::vector<Move> &moves)
{
}