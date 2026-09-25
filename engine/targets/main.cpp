#include "position.h"
#include "movegen.h"
#include "move.h"
#include "utils.h"

#include <iostream>
#include <vector>

using namespace std;

int main() {
    Position test = Position();

    test.whiteBishop = RankFiletoBB(2, 4);
    test.whiteKing = InttoBB(13);

    test.display();

    MoveGen handler;

    vector<Move> moves = handler.GenerateMoves(test, WHITE);

    u64 possibleMoves = 0;

    for (auto &&i : moves) {
        possibleMoves |= InttoBB(i.to);
    }

    test.whitePawn = possibleMoves;

    test.display();
}