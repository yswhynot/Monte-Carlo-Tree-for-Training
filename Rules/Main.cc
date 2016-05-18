#include "Board.h"
#include "Ann.h"

#include <iostream>
using namespace std;

/** Function prototypes **/
void testCases();

int main() {
    testCases();
    return 0;
}

void testCases() {
    /** Testing for Board **/
    Board board;
    // Test update and print
    int winner;
    winner = board.updateBoardByCommands(string("@0+ A0\\ A3/ @2\\ @2\\ A1\\ D2\\ E2\\ D1+ B4+ B5\\ @3/ @1+ H1\\ F0/ I1\\ @4/ C5/ C6/ C7/ @3\\ A2\\ A1+ @3+ G0+ E1+ D1+ D0\\ F10+ H9/ C8+ A8+ @6\\ F11+ F12/ E11/ D11/ D12/ B3+ B2\\ A2\\ @5/ @8+ B9/ B10+ F0/ I2/ @8/ M2/ N2/ K1/"));
    board.printType();
    cout << "Player "<< winner << " wins the game.\n";
    // Test check valid positions
    board.reset();
    winner = board.updateBoardByCommands(string("@0+ A0\\ A3/"));
    board.printType();
    int pos[TILENUM][4];
    int cnt;
    board.getValidPos(pos, &cnt);
    for (int i = 0; i < cnt; i++) {
        cout << pos[i][0] << ": " << (pos[i][1] == 1? "+": "") << (pos[i][2] == 1? "/": "") << (pos[i][3] == 1? "\\": "") << "\n";
    }
    cout << "\n";
    // Test output TileInfo
    TileInfo* tileInfos;
    tileInfos = board.getTileInfos(true);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
            cout << i << ": " << tileInfos[i].deltaRow << " " << tileInfos[i].deltaCol << " " << tileInfos[i].angle << "\n";
        }
    }
    cout << "\n";
    tileInfos = board.getTileInfos(false);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
            cout << i << ": " << tileInfos[i].deltaRow << " " << tileInfos[i].deltaCol << " " << tileInfos[i].angle << "\n";
        }
    }
    // Test getPathsFromBitset()
    int paths[ALLDIM];
    board.getPathsFromBitset(paths);
    /** Testing for Ann **/
    /*
    int x[7][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0}};
    int y[7][3] = {{0,0,0},{1,1,1},{0,0,0},{1,1,1},{0,0,0},{1,1,1},{0,0,0}};

    int** xx = new int*[7];
    int** yy = new int*[7];

    for (int i = 0; i < 7; i++) {
        xx[i] = x[i];
        yy[i] = y[i];
    }

    Ann ann(3);
    ann.training(xx, yy, 7);
    ann.print();

    delete [] xx;
    delete [] yy;
    */
    return;
}
