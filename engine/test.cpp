#include "position.h"
#include "iostream"
#include <vector>
using namespace std;

void displayBitboard(u64 bb) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            u64 bit = 1ULL << square;

            cout << ((bb & bit) ? "1 " : ". ");
        }

        cout << '\n';
    }

    cout << '\n';
}

void implementation1(){
    int moves[8] = {8+8+1, 8+8-1, -8-8+1, -8-8-1, 1+1+8, 1+1-8, -1-1-8, -1-1+8};
    Position testpos;
    testpos.whiteKnight = A1;
    testpos.whiteKnight |= A1<<(8+8+8+8+4);
    u64 allmoves = testpos.whiteKnight;
    for (auto i : moves)
    {
        u64 futureBitboard;
        if(i>0){
            futureBitboard = testpos.whiteKnight<<i;
        } else if(i<0){
            futureBitboard = testpos.whiteKnight>>-i;
        }
        allmoves|=futureBitboard;
    }
    displayBitboard(allmoves);
    testpos.display();
}


int main(){
    Position testpos;
    testpos.whiteKnight = A1;
    testpos.whiteKnight |= A1<<(8+8+8+8+4);

    vector<int> squares;
    for(u64 i = 1ULL; i!=0;i<<=1){
        if(testpos.whiteKnight & i){
            squares.emplace_back(__builtin_ctzll(i));
        }
    }

    int moves[8][2] = {
        {2,1},
        {2,-1},
        {1,-2},
        {-1,-2},
        {1,2},
        {-1,2},
        {-2,1},
        {-2,-1}
    };

    for (auto &&i : squares)
    {
        int rank = i%8;
        int file = i/8;
        
        
        
    }
    
    
    
    testpos.display();

}