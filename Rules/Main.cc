#include "Board.h"
#include "Ann.h"


int main() {
    /** Testing for Board **/
    Board board;
    board.updateBoard(0,'/');
    board.updateBoard(1,'\\');
    board.updateBoard(20,'\\');
    board.print();
    /** Testing for Ann **/
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

    return 0;
}
