#include <iostream>
#include <string.h>
#include <sstream>
#include <cmath>
using namespace std;

#include <bitset>
#include <vector>
#include "TileInfo.h"
using namespace std;

#ifndef BOARD_H_
#define BOARD_H_

#define BOARDWIDTH 20
#define TILENUM 400 // BOARDWIDTH * BOARDWIDTH
#define DIM 1200 // TILENUM * 3
#define ALLDIM 1600 // TILENUM * 4
#define LINEGAP 8
#define OUTPUTWIDTH 41 // BOARDWIDTH * 2 + 1

class Board {
public:
    Board();
    void reset();
    int updateBoardByCommands(string cmds);
    bool updateBoardByCommand(string cmd, int* winner);
    bool updateBoard(int pos, char type, int* winner);
    void printType();
    void printBit();
    TileInfo* getTileInfos(bool white);
    bool checkValid(int pos, char type);
    void getValidPos(int pos[TILENUM][4], int* posCnt, int* choiceCnt);
    void getPathsFromBitset(int paths[ALLDIM]);
    bitset<DIM> getBoardBitset();
    vector<string> getCmds();
	void boardConverter(bool* white, bool* red);
	void imageOutput(unsigned char* imageWhite, unsigned char* imageRed);
	void mapOutput(unsigned char* map);
	void loadBoardFromString(string state);
	void clockwise();
	void flip();
private:
    /** member functions **/
    bool forcePlay(int pos);
    void checkFourNeighbours(int row, int col, bool* topFlag, bool* leftFlag, bool* bottomFlag, bool* rightFlag);
    bool singleTileUpdate(int pos, char type);
    void shiftTempBoardBitset(int pos);
    int getRightEdge(int bitStart);
	void saveCmd(int pos, char type);
    /** member variables **/
    bool m_start;
    int m_winner; // 0: game is not end; 1: player 1 wins; 2: player 2 wins
	bool m_nowWhite;
    bitset<DIM> m_boardBitset;
    bitset<DIM> m_tempBoardBitset;
    int m_maxRow;
    int m_maxCol;
    int m_tempMaxRow;
    int m_tempMaxCol;
    int m_paths[ALLDIM]; // Store bit ID plus 1
    int m_tempPaths[ALLDIM];
    TileInfo m_tileInfos[TILENUM];
    vector<string> m_cmds;
};

#endif /* BOARD_H_ */
