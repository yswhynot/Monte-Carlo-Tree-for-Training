#include "Board.h"
#include "Ann.h"

#include <iostream>
using namespace std;

int main() {
    /** Testing for Board **/
    Board board;
    // Test update and print
    int winner;
    winner = board.updateBoardByCommands(string("@0/ A0/ A0/ @1/ B0/ C3\\ D3+ D0\\ @2\\ C1+ A4/ @4/ F5/"));
    board.printType();
    cout << "Player "<< winner << " wins the game.\n";
    // Test check valid positions
    board.reset();
    winner = board.updateBoardByCommands(string("@0/ A0/ A0/ @1/"));
    board.printType();
    int pos[TILENUM];
    int cnt;
    board.getValidPos(pos, &cnt);
    for (int i = 0; i < cnt; i++) {
        cout << pos[i] << " ";
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

    return 0;
}
