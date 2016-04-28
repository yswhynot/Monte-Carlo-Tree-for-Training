#include <bitset>
using namespace std;

#ifndef BOARD_H_
#define BOARD_H_

#define BOARDWIDTH 20
#define DIM 1200

class Board {
public:
    bool forcePlay(int pos);
    bool updateBoard(int pos, char type);
private:
    bitset<DIM> m_boardBitset;
    bitset<DIM> m_tempBoardBitset;
    int m_rowNum;
    int m_colNum;
    int getRightEdge(int bitStart);
};

#endif /* BOARD_H_ */
