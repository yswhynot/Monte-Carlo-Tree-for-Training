#include <bitset>
using namespace std;

#ifndef BOARD_H_
#define BOARD_H_

#define BOARDWIDTH 20
#define DIM 1200
#define LINEGAP 8

class Board {
public:
    Board();
    void reset();
    bool updateBoard(int pos, char type, int* winner);
    void printType();
    void printBit();
private:
    /** member functions **/
    bool forcePlay(int pos);
    bool singleTileUpdate(int pos, char type);
    void shiftTempBoardBitset(int pos);
    int getRightEdge(int bitStart);
    /** member variables **/
    bool m_start;
    int m_winner; // 0: game is not end; 1: player 1 wins; 2: player 2 wins
    bitset<DIM> m_boardBitset;
    bitset<DIM> m_tempBoardBitset;
    int m_maxRow;
    int m_maxCol;
    int m_tempMaxRow;
    int m_tempMaxCol;
    int m_paths[BOARDWIDTH * BOARDWIDTH * 4]; // Store bit ID plus 1
};

#endif /* BOARD_H_ */
