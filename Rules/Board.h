#include <bitset>
using namespace std;

#ifndef BOARD_H_
#define BOARD_H_

#define BOARDWIDTH 20
#define DIM 1200

class Board {
public:
    Board();
    bool updateBoard(int pos, char type);
    void print();
private:
    /** member functions **/
    bool forcePlay(int pos);
    bool singleTileUpdate(int pos, char type);
    int getRightEdge(int bitStart);
    /** member variables **/
    bool start;
    bitset<DIM> m_boardBitset;
    bitset<DIM> m_tempBoardBitset;
    int m_rowNum;
    int m_colNum;
};

#endif /* BOARD_H_ */
