
#include "iostream"
#include "move.h"
#include "movegen.h"
#include "position.h"
#include "utils.h"
#include <tuple>
#include <vector>
using namespace std;

int main() {
    Position test = Position();
    test.whiteBishop = RankFiletoBB(1,4);
    test.blackRook = RankFiletoBB(1,1)|RankFiletoBB(2,1);
    test.whiteKing = InttoBB(13);
    test.display();
    MoveGen handler;
    cout << "starting" << endl;
    vector<Move> moves = handler.GenerateMoves(test, WHITE);
    u64 possibleMoves = 0;

    for (auto &&i : moves) {
        auto sq = InttoBB(i.to);
        possibleMoves = possibleMoves|sq;
    }

    test.whitePawn = possibleMoves;
    test.display();
}