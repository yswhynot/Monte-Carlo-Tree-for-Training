#include "Board.h"

#include <iostream>
#include <string.h>
using namespace std;

Board::Board() {
    this->reset();
}

void Board::reset() {
    // Reset start and end
    this->m_start = true;
    this->m_winner = 0;
    // Reset board
    this->m_boardBitset.reset();
    // Reset paths
    for (int i = 0; i < BOARDWIDTH * BOARDWIDTH * 4; i++) {
        this->m_paths[i] = 0;
    }
}

int Board::updateBoardByCommands(string cmds) {
    int winner;
    int last = 0;
    for (int i = 0; i < cmds.length(); i++) {
        if (cmds[i] - ' ' == 0) {
            if (!this->updateBoardByCommand(cmds.substr(last, i - last), &winner)) {
                return -1;
            }
            last = i + 1;
        }
        if (i == cmds.length() - 1) {
            if (!this->updateBoardByCommand(cmds.substr(last), &winner)) {
                return -1;
            }
        }
    }
    return winner;
}

bool Board::updateBoardByCommand(string cmd, int* winner) {
    int cnt;
    // Handle column notation
    int col = 0;
    if (cmd[0] - '@' == 0) {
        cnt = 1;
    } else {
        for (int i = 0; i < cmd.length() - 1; i++) {
            if (cmd[i] >= 'A' && cmd[i] <= 'Z') {
                col *= 26;
                col += cmd[i] - 'A' + 1;
            } else {
                cnt = i;
                break;
            }
        }
    }
    // Handle row notation
    int row = 0;
    for (int i = cnt; i < cmd.length() - 1; i++) {
        row *= 10;
        row += cmd[i] - '0';
    }
    // Handle type
    char type = cmd[cmd.length() - 1];
    // Calculate position
    int pos = row * BOARDWIDTH + col;
    // Update board
    return this->updateBoard(pos, type, winner);
}

bool Board::updateBoard(int pos, char type, int* winner) {
    /* Check if the game is end */
    if (this->m_winner != 0) {
        *winner = this->m_winner;
        return false;
    }
    /* Backup current board */
    this->m_tempBoardBitset = this->m_boardBitset;
    this->m_tempMaxRow = this->m_maxRow;
    this->m_tempMaxCol = this->m_maxCol;
    for (int i = 0; i < ALLDIM; i++) {
        this->m_tempPaths[i] = this->m_paths[i];
    }
    /* Perform single-tile-update and force play */
    if (this->singleTileUpdate(pos, type)) {
        this->shiftTempBoardBitset(pos); // Shift the board
        this->m_boardBitset = this->m_tempBoardBitset; // Update the board
        this->m_maxRow = this->m_tempMaxRow;
        this->m_maxCol = this->m_tempMaxCol;
        for (int i = 0; i < ALLDIM; i++) {
            this->m_paths[i] = this->m_tempPaths[i];
        }
        *winner = this->m_winner; // Output winner
        return true;
    } else {
        *winner = 0; // Output winner
        return false;
    }
}

bool Board::singleTileUpdate(int pos, char type) {
    /* If the tile is the first one, must place in the first tile */
    if (this->m_start) {
        switch (type) {
            case '+':
                this->m_tempBoardBitset[0] = 0;
                this->m_tempBoardBitset[1] = 1;
                this->m_tempBoardBitset[2] = 0;
                // Update paths
                this->m_tempPaths[0] = 3; // 2 + 1
                this->m_tempPaths[1] = 4; // 3 + 1
                this->m_tempPaths[2] = 1; // 0 + 1
                this->m_tempPaths[3] = 2; // 1 + 1
                break;
            case '/':
                this->m_tempBoardBitset[0] = 0;
                this->m_tempBoardBitset[1] = 0;
                this->m_tempBoardBitset[2] = 1;
                // Update paths
                this->m_tempPaths[0] = 2; // 1 + 1
                this->m_tempPaths[1] = 1; // 0 + 1
                this->m_tempPaths[2] = 4; // 3 + 1
                this->m_tempPaths[3] = 3; // 2 + 1
                break;
            default: return false;
        }
        // Clear start flag
        this->m_start = false;
        // Update maxRow and maxCol
        this->m_tempMaxRow = 0;
        this->m_tempMaxCol = 0;
        return true;
    }
    /* Check if current tile is not empty */
    int bitStart = pos * 3;
    if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
        this->m_tempBoardBitset.test(bitStart + 2)) {
        return false;
    }
    /* Get four neighboring edges */
    int row = pos / BOARDWIDTH;
    int col = pos % BOARDWIDTH;
    // Update maxRow and maxCol
    if (row > this->m_tempMaxRow) {
        this->m_tempMaxRow = row;
    }
    if (col > this->m_tempMaxCol) {
        this->m_tempMaxCol = col;
    }

    int topPos = (pos - BOARDWIDTH) * 3;
    int leftPos = (pos - 1) * 3;
    int bottomPos = (pos + BOARDWIDTH) * 3;
    int rightPos = (pos + 1) * 3;

    bool topFlag = false, leftFlag = false, bottomFlag = false, rightFlag = false; // Set if there is tile

    bitset<4> whiteBits, redBits;

    // Check top
    if (row >= 1) {
        if (this->m_tempBoardBitset.test(topPos) || this->m_tempBoardBitset.test(topPos + 1) ||
            this->m_tempBoardBitset.test(topPos + 2)) {
            topFlag = true;
            whiteBits[0] = 1 - this->m_tempBoardBitset[topPos + 2];
            redBits[0] = this->m_tempBoardBitset[topPos + 2];
        }
    }

    // Check left
    if (col >= 1) {
        if (this->m_tempBoardBitset.test(leftPos) || this->m_tempBoardBitset.test(leftPos + 1) ||
            this->m_tempBoardBitset.test(leftPos + 2)) {
            leftFlag = true;
            whiteBits[1] = 1 - this->m_tempBoardBitset[leftPos + getRightEdge(leftPos)];
            redBits[1] = this->m_tempBoardBitset[leftPos + getRightEdge(leftPos)];
        }
    }

    // Check bottom
    if (row + 1 < BOARDWIDTH) {
        if (this->m_tempBoardBitset.test(bottomPos) || this->m_tempBoardBitset.test(bottomPos + 1) ||
            this->m_tempBoardBitset.test(bottomPos + 2)) {
            bottomFlag = true;
            whiteBits[2] = 1 - this->m_tempBoardBitset[bottomPos];
            redBits[2] = this->m_tempBoardBitset[bottomPos];
        }
    }

    // Check right
    if (col + 1 < BOARDWIDTH) {
        if (this->m_tempBoardBitset.test(rightPos) || this->m_tempBoardBitset.test(rightPos + 1) ||
            this->m_tempBoardBitset.test(rightPos + 2)) {
            rightFlag = true;
            whiteBits[3] = 1 - this->m_tempBoardBitset[rightPos + 1];
            redBits[3] = this->m_tempBoardBitset[rightPos + 1];
        }
    }
    /* Check current position valid or not from four edges */
    // No neighbour
    if (whiteBits.count() == 0 && redBits.count() == 0) {
        return false;
    }
    // Three same colors of neighbours
    if (whiteBits.count() >= 3 || redBits.count() >= 3) {
        return false;
    }
    /* Check if the type is valid */
    switch (type) {
        case '+':
            if ((whiteBits[0] != whiteBits[2]) && (whiteBits[1] != whiteBits[3]) &&
                (redBits[0] != redBits[2]) && (redBits[1] != redBits[3])) {
                return false;
            } else {
                for (int edge = 0; edge < 4; edge++) {
                    if (whiteBits[edge] != 0) {
                        switch (edge) {
                        case 0: case 2:
                            this->m_tempBoardBitset[pos * 3] = 0;
                            this->m_tempBoardBitset[pos * 3 + 1] = 1;
                            this->m_tempBoardBitset[pos * 3 + 2] = 0;
                            break; // Case
                        case 1: case 3:
                            this->m_tempBoardBitset[pos * 3] = 1;
                            this->m_tempBoardBitset[pos * 3 + 1] = 0;
                            this->m_tempBoardBitset[pos * 3 + 2] = 1;
                            break; // Case
                        }
                        break; // For
                    } else if (redBits[edge] != 0) {
                        switch (edge) {
                        case 0: case 2:
                            this->m_tempBoardBitset[pos * 3] = 1;
                            this->m_tempBoardBitset[pos * 3 + 1] = 0;
                            this->m_tempBoardBitset[pos * 3 + 2] = 1;
                            break; // Case
                        case 1: case 3:
                            this->m_tempBoardBitset[pos * 3] = 0;
                            this->m_tempBoardBitset[pos * 3 + 1] = 1;
                            this->m_tempBoardBitset[pos * 3 + 2] = 0;
                            break; // Case
                        }
                        break; // For
                    }
                }
                /* Update Paths and check win-lose */
                int topPathPos = (pos - BOARDWIDTH) * 4;
                int leftPathPos = (pos - 1) * 4;
                int bottomPathPos = (pos + BOARDWIDTH) * 4;
                int rightPathPos = (pos + 1) * 4;
                // Check top and bottom
                if (topFlag && bottomFlag) {
                    // Check if win by loop
                    if ((this->m_tempPaths[topPathPos + 2] == bottomPathPos + 1) &&
                        (this->m_tempPaths[bottomPathPos] == topPathPos + 3)) {
                        // Game ends, check winner
                        if (this->m_tempBoardBitset.test(pos * 3)) {
                            this->m_winner = 2;
                        } else {
                            this->m_winner = 1;
                        }
                    } else {
                        /* Check if win by line */
                        int endpointOne = this->m_tempPaths[topPathPos + 2] - 1;
                        int edgeOne = endpointOne % 4;
                        int endpointTwo = this->m_tempPaths[bottomPathPos] - 1;
                        int edgeTwo = endpointTwo % 4;

                        int posOne = endpointOne / 4;
                        int posTwo = endpointTwo / 4;

                        // If top-bottom line
                        int minRow;
                        if (row == 0) {
                            minRow = 0;
                        } else {
                            minRow = 1;
                        }
                        if ((edgeOne == 0 && edgeTwo == 2) || (edgeOne == 2 && edgeTwo == 0)) {
                            int rowOne = posOne / BOARDWIDTH;
                            int rowTwo = posTwo / BOARDWIDTH;
                            int rowGap;
                            bool rowSpan = false;
                            int checkPos;
                            if (rowOne > rowTwo) {
                                rowGap = rowOne - rowTwo;
                                rowSpan = (rowOne >= this->m_tempMaxRow) && (rowTwo <= minRow);
                                checkPos = posTwo;
                            } else {
                                rowGap = rowTwo - rowOne;
                                rowSpan = (rowTwo >= this->m_tempMaxRow) && (rowOne <= minRow);
                                checkPos = posOne;
                            }
                            if (rowSpan && rowGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        // If left-right line
                        int minCol;
                        if (col == 0) {
                            minCol = 0;
                        } else {
                            minCol = 1;
                        }
                        if ((edgeOne == 1 && edgeTwo == 3) || (edgeOne == 3 && edgeTwo == 1)) {
                            int colOne = posOne % BOARDWIDTH;
                            int colTwo = posTwo % BOARDWIDTH;
                            int colGap;
                            bool colSpan = false;
                            int checkPos;
                            if (colOne > colTwo) {
                                colGap = colOne - colTwo;
                                colSpan = (colOne >= this->m_tempMaxCol) && (colTwo <= minCol);
                                checkPos = colTwo;
                            } else {
                                colGap = colTwo - colOne;
                                colSpan = (colTwo >= this->m_tempMaxCol) && (colOne <= minCol);
                                checkPos = colOne;
                            }
                            if (colSpan && colGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3 + 1)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        /* Merge two paths */
                        this->m_tempPaths[this->m_tempPaths[topPathPos + 2] - 1] = this->m_tempPaths[bottomPathPos];
                        this->m_tempPaths[this->m_tempPaths[bottomPathPos] - 1] = this->m_tempPaths[topPathPos + 2];
                        this->m_tempPaths[topPathPos + 2] = 0;
                        this->m_tempPaths[bottomPathPos] = 0;
                    }
                } else if (topFlag) {
                    this->m_tempPaths[this->m_tempPaths[topPathPos + 2] - 1] = pos * 4 + 3;
                    this->m_tempPaths[pos * 4 + 2] = this->m_tempPaths[topPathPos + 2];
                    this->m_tempPaths[topPathPos + 2] = 0;
                } else if (bottomFlag) {
                    this->m_tempPaths[this->m_tempPaths[bottomPathPos] - 1] = pos * 4 + 1;
                    this->m_tempPaths[pos * 4] = this->m_tempPaths[bottomPathPos];
                    this->m_tempPaths[bottomPathPos] = 0;
                } else {
                    this->m_tempPaths[pos * 4 + 2] = pos * 4 + 1;
                    this->m_tempPaths[pos * 4] = pos * 4 + 3;
                }
                // Check left and right
                if (leftFlag && rightFlag) {
                    // Check if win by loop
                    if ((this->m_tempPaths[leftPathPos + 3] == rightPathPos + 2) &&
                        (this->m_tempPaths[rightPathPos + 1] == leftPathPos + 4)) {
                        // Game ends, check winner
                        if (this->m_tempBoardBitset.test(pos * 3 + 1)) {
                            this->m_winner = 2;
                        } else {
                            this->m_winner = 1;
                        }
                    } else {
                        /* Check if win by line */
                        int endpointOne = this->m_tempPaths[leftPathPos + 3] - 1;
                        int edgeOne = endpointOne % 4;
                        int endpointTwo = this->m_tempPaths[rightPathPos + 1] - 1;
                        int edgeTwo = endpointTwo % 4;

                        int posOne = endpointOne / 4;
                        int posTwo = endpointTwo / 4;

                        // If top-bottom line
                        int minRow;
                        if (row == 0) {
                            minRow = 0;
                        } else {
                            minRow = 1;
                        }
                        if ((edgeOne == 0 && edgeTwo == 2) || (edgeOne == 2 && edgeTwo == 0)) {
                            int rowOne = posOne / BOARDWIDTH;
                            int rowTwo = posTwo / BOARDWIDTH;
                            int rowGap;
                            bool rowSpan = false;
                            int checkPos;
                            if (rowOne > rowTwo) {
                                rowGap = rowOne - rowTwo;
                                rowSpan = (rowOne >= this->m_tempMaxRow) && (rowTwo <= minRow);
                                checkPos = rowTwo;
                            } else {
                                rowGap = rowTwo - rowOne;
                                rowSpan = (rowTwo >= this->m_tempMaxRow) && (rowOne <= minRow);
                                checkPos = rowOne;
                            }
                            if (rowSpan && rowGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        // If left-right line
                        int minCol;
                        if (col == 0) {
                            minCol = 0;
                        } else {
                            minCol = 1;
                        }
                        if ((edgeOne == 1 && edgeTwo == 3) || (edgeOne == 3 && edgeTwo == 1)) {
                            int colOne = posOne % BOARDWIDTH;
                            int colTwo = posTwo % BOARDWIDTH;
                            int colGap;
                            bool colSpan = false;
                            int checkPos;
                            if (colOne > colTwo) {
                                colGap = colOne - colTwo;
                                colSpan = (colOne >= this->m_tempMaxCol) && (colTwo <= minCol);
                                checkPos = colTwo;
                            } else {
                                colGap = colTwo - colOne;
                                colSpan = (colTwo >= this->m_tempMaxCol) && (colOne <= minCol);
                                checkPos = colOne;
                            }
                            if (colSpan && colGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3 + 1)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        /* Merge two paths */
                        this->m_tempPaths[this->m_tempPaths[leftPathPos + 3] - 1] = this->m_tempPaths[rightPathPos + 1];
                        this->m_tempPaths[this->m_tempPaths[rightPathPos + 1] - 1] = this->m_tempPaths[leftPathPos + 3];
                        this->m_tempPaths[leftPathPos + 3] = 0;
                        this->m_tempPaths[rightPathPos + 1] = 0;
                    }
                } else if (leftFlag) {
                    this->m_tempPaths[this->m_tempPaths[leftPathPos + 3] - 1] = pos * 4 + 4;
                    this->m_tempPaths[pos * 4 + 3] = this->m_tempPaths[leftPathPos + 3];
                    this->m_tempPaths[leftPathPos + 3] = 0;
                } else if (rightFlag) {
                    this->m_tempPaths[this->m_tempPaths[rightPathPos + 1] - 1] = pos * 4 + 2;
                    this->m_tempPaths[pos * 4 + 1] = this->m_tempPaths[rightPathPos + 1];
                    this->m_tempPaths[rightPathPos + 1] = 0;
                } else {
                    this->m_tempPaths[pos * 4 + 3] = pos * 4 + 2;
                    this->m_tempPaths[pos * 4 + 1] = pos * 4 + 4;
                }
            }
            break;
        case '\\':
            if ((whiteBits[0] != whiteBits[3]) && (whiteBits[1] != whiteBits[2]) &&
                (redBits[0] != redBits[3]) && (redBits[1] != redBits[2])) {
                return false;
            } else {
                for (int edge = 0; edge < 4; edge++) {
                    if (whiteBits[edge] != 0) {
                        switch (edge) {
                        case 0: case 3:
                            this->m_tempBoardBitset[pos * 3] = 0;
                            this->m_tempBoardBitset[pos * 3 + 1] = 1;
                            this->m_tempBoardBitset[pos * 3 + 2] = 1;
                            break; // Case
                        case 1: case 2:
                            this->m_tempBoardBitset[pos * 3] = 1;
                            this->m_tempBoardBitset[pos * 3 + 1] = 0;
                            this->m_tempBoardBitset[pos * 3 + 2] = 0;
                            break; // Case
                        }
                        break; // For
                    } else if (redBits[edge] != 0) {
                        switch (edge) {
                        case 0: case 3:
                            this->m_tempBoardBitset[pos * 3] = 1;
                            this->m_tempBoardBitset[pos * 3 + 1] = 0;
                            this->m_tempBoardBitset[pos * 3 + 2] = 0;
                            break; // Case
                        case 1: case 2:
                            this->m_tempBoardBitset[pos * 3] = 0;
                            this->m_tempBoardBitset[pos * 3 + 1] = 1;
                            this->m_tempBoardBitset[pos * 3 + 2] = 1;
                            break; // Case
                        }
                        break; // For
                    }
                }
                /* Update Paths and check win-lose */
                int topPathPos = (pos - BOARDWIDTH) * 4;
                int leftPathPos = (pos - 1) * 4;
                int bottomPathPos = (pos + BOARDWIDTH) * 4;
                int rightPathPos = (pos + 1) * 4;
                // Check top and right
                if (topFlag && rightFlag) {
                    // Check if win by loop
                    if ((this->m_tempPaths[topPathPos + 2] == rightPathPos + 2) &&
                        (this->m_tempPaths[rightPathPos + 1] == topPathPos + 3)) {
                        // Game ends, check winner
                        if (this->m_tempBoardBitset.test(pos * 3)) {
                            this->m_winner = 2;
                        } else {
                            this->m_winner = 1;
                        }
                    } else {
                        /* Check if win by line */
                        int endpointOne = this->m_tempPaths[topPathPos + 2] - 1;
                        int edgeOne = endpointOne % 4;
                        int endpointTwo = this->m_tempPaths[rightPathPos + 1] - 1;
                        int edgeTwo = endpointTwo % 4;

                        int posOne = endpointOne / 4;
                        int posTwo = endpointTwo / 4;

                        // If top-bottom line
                        int minRow;
                        if (row == 0) {
                            minRow = 0;
                        } else {
                            minRow = 1;
                        }
                        if ((edgeOne == 0 && edgeTwo == 2) || (edgeOne == 2 && edgeTwo == 0)) {
                            int rowOne = posOne / BOARDWIDTH;
                            int rowTwo = posTwo / BOARDWIDTH;
                            int rowGap;
                            bool rowSpan = false;
                            int checkPos;
                            if (rowOne > rowTwo) {
                                rowGap = rowOne - rowTwo;
                                rowSpan = (rowOne >= this->m_tempMaxRow) && (rowTwo <= minRow);
                                checkPos = rowTwo;
                            } else {
                                rowGap = rowTwo - rowOne;
                                rowSpan = (rowTwo >= this->m_tempMaxRow) && (rowOne <= minRow);
                                checkPos = rowOne;
                            }
                            if (rowSpan && rowGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        // If left-right line
                        int minCol;
                        if (col == 0) {
                            minCol = 0;
                        } else {
                            minCol = 1;
                        }
                        if ((edgeOne == 1 && edgeTwo == 3) || (edgeOne == 3 && edgeTwo == 1)) {
                            int colOne = posOne % BOARDWIDTH;
                            int colTwo = posTwo % BOARDWIDTH;
                            int colGap;
                            bool colSpan = false;
                            int checkPos;
                            if (colOne > colTwo) {
                                colGap = colOne - colTwo;
                                colSpan = (colOne >= this->m_tempMaxCol) && (colTwo <= minCol);
                                checkPos = colTwo;
                            } else {
                                colGap = colTwo - colOne;
                                colSpan = (colTwo >= this->m_tempMaxCol) && (colOne <= minCol);
                                checkPos = colOne;
                            }
                            if (colSpan && colGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3 + 1)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        /* Merge two paths */
                        this->m_tempPaths[this->m_tempPaths[topPathPos + 2] - 1] = this->m_tempPaths[rightPathPos + 1];
                        this->m_tempPaths[this->m_tempPaths[rightPathPos + 1] - 1] = this->m_tempPaths[topPathPos + 2];
                        this->m_tempPaths[topPathPos + 2] = 0;
                        this->m_tempPaths[rightPathPos + 1] = 0;
                    }
                } else if (topFlag) {
                    this->m_tempPaths[this->m_tempPaths[topPathPos + 2] - 1] = pos * 4 + 4;
                    this->m_tempPaths[pos * 4 + 3] = this->m_tempPaths[topPathPos + 2];
                    this->m_tempPaths[topPathPos + 2] = 0;
                } else if (rightFlag) {
                    this->m_tempPaths[this->m_tempPaths[rightPathPos + 1] - 1] = pos * 4 + 1;
                    this->m_tempPaths[pos * 4] = this->m_tempPaths[rightPathPos + 1];
                    this->m_tempPaths[rightPathPos + 1] = 0;
                } else {
                    this->m_tempPaths[pos * 4 + 3] = pos * 4 + 1;
                    this->m_tempPaths[pos * 4] = pos * 4 + 4;
                }
                // Check left and bottom
                if (leftFlag && bottomFlag) {
                    // Check if win by loop
                    if ((this->m_tempPaths[leftPathPos + 3] == bottomPathPos + 1) &&
                        (this->m_tempPaths[bottomPathPos] == leftPathPos + 4)) {
                        // Game ends, check winner
                        if (this->m_tempBoardBitset.test(pos * 3 + 1)) {
                            this->m_winner = 2;
                        } else {
                            this->m_winner = 1;
                        }
                    } else {
                        /* Check if win by line */
                        int endpointOne = this->m_tempPaths[leftPathPos + 3] - 1;
                        int edgeOne = endpointOne % 4;
                        int endpointTwo = this->m_tempPaths[bottomPathPos] - 1;
                        int edgeTwo = endpointTwo % 4;

                        int posOne = endpointOne / 4;
                        int posTwo = endpointTwo / 4;

                        // If top-bottom line
                        int minRow;
                        if (row == 0) {
                            minRow = 0;
                        } else {
                            minRow = 1;
                        }
                        if ((edgeOne == 0 && edgeTwo == 2) || (edgeOne == 2 && edgeTwo == 0)) {
                            int rowOne = posOne / BOARDWIDTH;
                            int rowTwo = posTwo / BOARDWIDTH;
                            int rowGap;
                            bool rowSpan = false;
                            int checkPos;
                            if (rowOne > rowTwo) {
                                rowGap = rowOne - rowTwo;
                                rowSpan = (rowOne >= this->m_tempMaxRow) && (rowTwo <= minRow);
                                checkPos = rowTwo;
                            } else {
                                rowGap = rowTwo - rowOne;
                                rowSpan = (rowTwo >= this->m_tempMaxRow) && (rowOne <= minRow);
                                checkPos = rowOne;
                            }
                            if (rowSpan && rowGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        // If left-right line
                        int minCol;
                        if (col == 0) {
                            minCol = 0;
                        } else {
                            minCol = 1;
                        }
                        if ((edgeOne == 1 && edgeTwo == 3) || (edgeOne == 3 && edgeTwo == 1)) {
                            int colOne = posOne % BOARDWIDTH;
                            int colTwo = posTwo % BOARDWIDTH;
                            int colGap;
                            bool colSpan = false;
                            int checkPos;
                            if (colOne > colTwo) {
                                colGap = colOne - colTwo;
                                colSpan = (colOne >= this->m_tempMaxCol) && (colTwo <= minCol);
                                checkPos = colTwo;
                            } else {
                                colGap = colTwo - colOne;
                                colSpan = (colTwo >= this->m_tempMaxCol) && (colOne <= minCol);
                                checkPos = colOne;
                            }
                            if (colSpan && colGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3 + 1)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        /* Merge two paths */
                        this->m_tempPaths[this->m_tempPaths[leftPathPos + 3] - 1] = this->m_tempPaths[bottomPathPos];
                        this->m_tempPaths[this->m_tempPaths[bottomPathPos] - 1] = this->m_tempPaths[leftPathPos + 3];
                        this->m_tempPaths[leftPathPos + 3] = 0;
                        this->m_tempPaths[bottomPathPos] = 0;
                    }
                } else if (leftFlag) {
                    this->m_tempPaths[this->m_tempPaths[leftPathPos + 3] - 1] = pos * 4 + 3;
                    this->m_tempPaths[pos * 4 + 2] = this->m_tempPaths[leftPathPos + 3];
                    this->m_tempPaths[leftPathPos + 3] = 0;
                } else if (bottomFlag) {
                    this->m_tempPaths[this->m_tempPaths[bottomPathPos] - 1] = pos * 4 + 2;
                    this->m_tempPaths[pos * 4 + 1] = this->m_tempPaths[bottomPathPos];
                    this->m_tempPaths[bottomPathPos] = 0;
                } else {
                    this->m_tempPaths[pos * 4 + 2] = pos * 4 + 2;
                    this->m_tempPaths[pos * 4 + 1] = pos * 4 + 3;
                }
            }
            break;
        case '/':
            if ((whiteBits[0] != whiteBits[1]) && (whiteBits[2] != whiteBits[3]) &&
                (redBits[0] != redBits[1]) && (redBits[2] != redBits[3])) {
                return false;
            } else {
                for (int edge = 0; edge < 4; edge++) {
                    if (whiteBits[edge] != 0) {
                        switch (edge) {
                        case 0: case 1:
                            this->m_tempBoardBitset[pos * 3] = 0;
                            this->m_tempBoardBitset[pos * 3 + 1] = 0;
                            this->m_tempBoardBitset[pos * 3 + 2] = 1;
                            break; // Case
                        case 2: case 3:
                            this->m_tempBoardBitset[pos * 3] = 1;
                            this->m_tempBoardBitset[pos * 3 + 1] = 1;
                            this->m_tempBoardBitset[pos * 3 + 2] = 0;
                            break; // Case
                        }
                        break; // For
                    } else if (redBits[edge] != 0) {
                        switch (edge) {
                        case 0: case 1:
                            this->m_tempBoardBitset[pos * 3] = 1;
                            this->m_tempBoardBitset[pos * 3 + 1] = 1;
                            this->m_tempBoardBitset[pos * 3 + 2] = 0;
                            break; // Case
                        case 2: case 3:
                            this->m_tempBoardBitset[pos * 3] = 0;
                            this->m_tempBoardBitset[pos * 3 + 1] = 0;
                            this->m_tempBoardBitset[pos * 3 + 2] = 1;
                            break; // Case
                        }
                        break; // For
                    }
                }
                /* Update Paths and check win-lose */
                int topPathPos = (pos - BOARDWIDTH) * 4;
                int leftPathPos = (pos - 1) * 4;
                int bottomPathPos = (pos + BOARDWIDTH) * 4;
                int rightPathPos = (pos + 1) * 4;
                // Check top and left
                if (topFlag && leftFlag) {
                    // Check if win by loop
                    if ((this->m_tempPaths[topPathPos + 2] == leftPathPos + 4) &&
                        (this->m_tempPaths[leftPathPos + 3] == topPathPos + 3)) {
                        // Game ends, check winner
                        if (this->m_tempBoardBitset.test(pos * 3)) {
                            this->m_winner = 2;
                        } else {
                            this->m_winner = 1;
                        }
                    } else {
                        /* Check if win by line */
                        int endpointOne = this->m_tempPaths[topPathPos + 2] - 1;
                        int edgeOne = endpointOne % 4;
                        int endpointTwo = this->m_tempPaths[leftPathPos + 3] - 1;
                        int edgeTwo = endpointTwo % 4;

                        int posOne = endpointOne / 4;
                        int posTwo = endpointTwo / 4;

                        // If top-bottom line
                        int minRow;
                        if (row == 0) {
                            minRow = 0;
                        } else {
                            minRow = 1;
                        }
                        if ((edgeOne == 0 && edgeTwo == 2) || (edgeOne == 2 && edgeTwo == 0)) {
                            int rowOne = posOne / BOARDWIDTH;
                            int rowTwo = posTwo / BOARDWIDTH;
                            int rowGap;
                            bool rowSpan = false;
                            int checkPos;
                            if (rowOne > rowTwo) {
                                rowGap = rowOne - rowTwo;
                                rowSpan = (rowOne >= this->m_tempMaxRow) && (rowTwo <= minRow);
                                checkPos = rowTwo;
                            } else {
                                rowGap = rowTwo - rowOne;
                                rowSpan = (rowTwo >= this->m_tempMaxRow) && (rowOne <= minRow);
                                checkPos = rowOne;
                            }
                            if (rowSpan && rowGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        // If left-right line
                        int minCol;
                        if (col == 0) {
                            minCol = 0;
                        } else {
                            minCol = 1;
                        }
                        if ((edgeOne == 1 && edgeTwo == 3) || (edgeOne == 3 && edgeTwo == 1)) {
                            int colOne = posOne % BOARDWIDTH;
                            int colTwo = posTwo % BOARDWIDTH;
                            int colGap;
                            bool colSpan = false;
                            int checkPos;
                            if (colOne > colTwo) {
                                colGap = colOne - colTwo;
                                colSpan = (colOne >= this->m_tempMaxCol) && (colTwo <= minCol);
                                checkPos = colTwo;
                            } else {
                                colGap = colTwo - colOne;
                                colSpan = (colTwo >= this->m_tempMaxCol) && (colOne <= minCol);
                                checkPos = colOne;
                            }
                            if (colSpan && colGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3 + 1)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        /* Merge two paths */
                        this->m_tempPaths[this->m_tempPaths[topPathPos + 2] - 1] = this->m_tempPaths[leftPathPos + 3];
                        this->m_tempPaths[this->m_tempPaths[leftPathPos + 3] - 1] = this->m_tempPaths[topPathPos + 2];
                        this->m_tempPaths[topPathPos + 2] = 0;
                        this->m_tempPaths[leftPathPos + 3] = 0;
                    }
                } else if (topFlag) {
                    this->m_tempPaths[this->m_tempPaths[topPathPos + 2] - 1] = pos * 4 + 2;
                    this->m_tempPaths[pos * 4 + 1] = this->m_tempPaths[topPathPos + 2];
                    this->m_tempPaths[topPathPos + 2] = 0;
                } else if (leftFlag) {
                    this->m_tempPaths[this->m_tempPaths[leftPathPos + 3] - 1] = pos * 4 + 1;
                    this->m_tempPaths[pos * 4] = this->m_tempPaths[leftPathPos + 3];
                    this->m_tempPaths[leftPathPos + 3] = 0;
                } else {
                    this->m_tempPaths[pos * 4 + 1] = pos * 4 + 1;
                    this->m_tempPaths[pos * 4] = pos * 4 + 2;
                }
                // Check bottom and right
                if (bottomFlag && rightFlag) {
                    // Check if win by loop
                    if ((this->m_tempPaths[bottomPathPos] == rightPathPos + 2) &&
                        (this->m_tempPaths[rightPathPos + 1] == bottomPathPos + 1)) {
                        // Game ends, check winner
                        if (this->m_tempBoardBitset.test(pos * 3 + 2)) {
                            this->m_winner = 2;
                        } else {
                            this->m_winner = 1;
                        }
                    } else {
                        /* Check if win by line */
                        int endpointOne = this->m_tempPaths[bottomPathPos] - 1;
                        int edgeOne = endpointOne % 4;
                        int endpointTwo = this->m_tempPaths[rightPathPos + 1] - 1;
                        int edgeTwo = endpointTwo % 4;

                        int posOne = endpointOne / 4;
                        int posTwo = endpointTwo / 4;

                        // If top-bottom line
                        int minRow;
                        if (row == 0) {
                            minRow = 0;
                        } else {
                            minRow = 1;
                        }
                        if ((edgeOne == 0 && edgeTwo == 2) || (edgeOne == 2 && edgeTwo == 0)) {
                            int rowOne = posOne / BOARDWIDTH;
                            int rowTwo = posTwo / BOARDWIDTH;
                            int rowGap;
                            bool rowSpan = false;
                            int checkPos;
                            if (rowOne > rowTwo) {
                                rowGap = rowOne - rowTwo;
                                rowSpan = (rowOne >= this->m_tempMaxRow) && (rowTwo <= minRow);
                                checkPos = rowTwo;
                            } else {
                                rowGap = rowTwo - rowOne;
                                rowSpan = (rowTwo >= this->m_tempMaxRow) && (rowOne <= minRow);
                                checkPos = rowOne;
                            }
                            if (rowSpan && rowGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        // If left-right line
                        int minCol;
                        if (col == 0) {
                            minCol = 0;
                        } else {
                            minCol = 1;
                        }
                        if ((edgeOne == 1 && edgeTwo == 3) || (edgeOne == 3 && edgeTwo == 1)) {
                            int colOne = posOne % BOARDWIDTH;
                            int colTwo = posTwo % BOARDWIDTH;
                            int colGap;
                            bool colSpan = false;
                            int checkPos;
                            if (colOne > colTwo) {
                                colGap = colOne - colTwo;
                                colSpan = (colOne >= this->m_tempMaxCol) && (colTwo <= minCol);
                                checkPos = colTwo;
                            } else {
                                colGap = colTwo - colOne;
                                colSpan = (colTwo >= this->m_tempMaxCol) && (colOne <= minCol);
                                checkPos = colOne;
                            }
                            if (colSpan && colGap >= LINEGAP) {
                                // Game wins, check winner
                                if (this->m_tempBoardBitset.test(checkPos * 3 + 1)) {
                                    this->m_winner = 2;
                                } else {
                                    this->m_winner = 1;
                                }
                            }
                        }
                        /* Merge two paths */
                        this->m_tempPaths[this->m_tempPaths[bottomPathPos] - 1] = this->m_tempPaths[rightPathPos + 1];
                        this->m_tempPaths[this->m_tempPaths[rightPathPos + 1] - 1] = this->m_tempPaths[bottomPathPos];
                        this->m_tempPaths[bottomPathPos] = 0;
                        this->m_tempPaths[rightPathPos + 1] = 0;
                    }
                } else if (bottomFlag) {
                    this->m_tempPaths[this->m_tempPaths[bottomPathPos] - 1] = pos * 4 + 4;
                    this->m_tempPaths[pos * 4 + 3] = this->m_tempPaths[bottomPathPos];
                    this->m_tempPaths[bottomPathPos] = 0;
                } else if (rightFlag) {
                    this->m_tempPaths[this->m_tempPaths[rightPathPos + 1] - 1] = pos * 4 + 3;
                    this->m_tempPaths[pos * 4 + 2] = this->m_tempPaths[rightPathPos + 1];
                    this->m_tempPaths[rightPathPos + 1] = 0;
                } else {
                    this->m_tempPaths[pos * 4 + 3] = pos * 4 + 3;
                    this->m_tempPaths[pos * 4 + 2] = pos * 4 + 4;
                }
            }
            break;
        default: return false;
    }
    // Check force play
    if (this->forcePlay(pos)) {
        return true;
    } else {
        this->m_winner = 0; // Reset winner
        return false;
    }
}

bool Board::forcePlay(int pos) {
    int row = pos / BOARDWIDTH;
    int col = pos % BOARDWIDTH;

    /* Check which position no need to force play (since has tile already) */
    bool topFlag = false, leftFlag = false, bottomFlag = false, rightFlag = false;
    this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);

    /* Check force play */
    // Check top
    if (row >= 2) {
        int bitStart = ((row - 2) * BOARDWIDTH + col) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tilt to check is not empty
            // Bottom of the tilt
            if (!topFlag) {
                if (this->m_tempBoardBitset[pos * 3] == this->m_tempBoardBitset[bitStart + 2]) {
                    if (singleTileUpdate((row - 1) * BOARDWIDTH + col, '+') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    // Check top-left
    if ((row >= 1) && (col >= 1)) {
        int bitStart = ((row - 1) * BOARDWIDTH + col - 1) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Right of the tilt
            if (!topFlag) {
                if (this->m_tempBoardBitset[pos * 3] == this->m_tempBoardBitset[bitStart + this->getRightEdge(bitStart)]) {
                    if (singleTileUpdate((row - 1) * BOARDWIDTH + col, '\\') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }

            // Bottom of the tilt
            if (!leftFlag) {
                if (this->m_tempBoardBitset[pos * 3 + 1] == this->m_tempBoardBitset[bitStart + 2]) {
                    if (singleTileUpdate(row * BOARDWIDTH + col - 1, '\\') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    // Check left
    if ((col >= 2)) {
        int bitStart = ((row * BOARDWIDTH + col - 2)) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Right of the tilt
            if (!leftFlag) {
                if (this->m_tempBoardBitset[pos * 3 + 1] == this->m_tempBoardBitset[bitStart + this->getRightEdge(bitStart)]) {
                    if (singleTileUpdate(row * BOARDWIDTH + col - 1, '+') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }

        }
    }
    // Check bottom-left
    if ((row + 1 < BOARDWIDTH) && (col >= 1)) {
        int bitStart = ((row + 1) * BOARDWIDTH + col - 1) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Top of the tilt
            if (!leftFlag) {
                if (this->m_tempBoardBitset[pos * 3 + 1] == this->m_tempBoardBitset[bitStart]) {
                    if (singleTileUpdate(row * BOARDWIDTH + col - 1, '/') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }

            // Right of the tilt
            if (!bottomFlag) {
                if (this->m_tempBoardBitset[pos * 3 + 2] == this->m_tempBoardBitset[bitStart + this->getRightEdge(bitStart)]) {
                    if (singleTileUpdate((row + 1) * BOARDWIDTH + col, '/') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    // Check bottom
    if ((row + 2 < BOARDWIDTH)) {
        int bitStart = ((row + 2) * BOARDWIDTH + col) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Top of the tilt
            if (!bottomFlag) {
                if (this->m_tempBoardBitset[pos * 3 + 2] == this->m_tempBoardBitset[bitStart]) {
                    if (singleTileUpdate((row + 1) * BOARDWIDTH + col, '+') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    // Check bottom-right
    if ((row + 1 < BOARDWIDTH) && (col + 1 < BOARDWIDTH)) {
        int bitStart = ((row + 1) * BOARDWIDTH + col + 1) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Left of the tilt
            if (!bottomFlag) {
                if (this->m_tempBoardBitset[pos * 3 + 2] == this->m_tempBoardBitset[bitStart + 1]) {
                    if (singleTileUpdate((row + 1) * BOARDWIDTH + col, '\\') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }

            // Top of the tilt
            if (!rightFlag) {
                if (this->m_tempBoardBitset[pos * 3 + this->getRightEdge(pos * 3)] == this->m_tempBoardBitset[bitStart]) {
                    if (singleTileUpdate(row * BOARDWIDTH + col + 1, '\\') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    // Check right
    if (col + 2 < BOARDWIDTH) {
        int bitStart = (row * BOARDWIDTH + col + 2) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Left of the tilt
            if (!rightFlag) {
                if (this->m_tempBoardBitset[pos * 3 + this->getRightEdge(pos * 3)] == this->m_tempBoardBitset[bitStart + 1]) {
                    if (singleTileUpdate(row * BOARDWIDTH + col + 1, '+') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    // Check top-right
    if ((row >= 1) && (col + 1 < BOARDWIDTH)) {
        int bitStart = ((row - 1) * BOARDWIDTH + col + 1) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) { // The tile to check is not empty
            // Bottom of the tilt
            if (!rightFlag) {
                if (this->m_tempBoardBitset[pos * 3 + this->getRightEdge(pos * 3)] == this->m_tempBoardBitset[bitStart + 2]) {
                    if (singleTileUpdate(row * BOARDWIDTH + col + 1, '/') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }

            // Left of the tilt
            if (!topFlag) {
                if (this->m_tempBoardBitset[pos * 3] == this->m_tempBoardBitset[bitStart + 1]) {
                    if (singleTileUpdate((row - 1) * BOARDWIDTH + col, '/') == false) {
                        return false;
                    } else {
                        this->checkFourNeighbours(row, col, &topFlag, &leftFlag, &bottomFlag, &rightFlag);
                    }
                }
            }
        }
    }
    return true;
}

bool Board::checkFourNeighbours(int row, int col, bool* topFlag, bool* leftFlag, bool* bottomFlag, bool* rightFlag) {
    // Check top
    if (row >= 1) {
        int bitStart = ((row - 1) * BOARDWIDTH + col) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) {
            *topFlag = true;
        }
    }
    // Check left
    if (col >= 1) {
        int bitStart = (row * BOARDWIDTH + col - 1) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) {
            *leftFlag = true;
        }
    }
    // Check bottom
    if (row + 1 < BOARDWIDTH) {
        int bitStart = ((row + 1) * BOARDWIDTH + col) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) {
            *bottomFlag = true;
        }
    }
    // Check right
    if (col + 1 < BOARDWIDTH) {
        int bitStart = (row * BOARDWIDTH + col + 1) * 3;
        if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
            this->m_tempBoardBitset.test(bitStart + 2)) {
            *rightFlag = true;
        }
    }
}

void Board::shiftTempBoardBitset(int pos) {
    int row = pos / BOARDWIDTH;
    int col = pos % BOARDWIDTH;

    if (row == 0) {
        // Shift board
        bitset<DIM> shiftedBoardBitset;
        for (int bit = 0; bit < DIM - BOARDWIDTH * 3; bit++) {
            shiftedBoardBitset[bit + BOARDWIDTH * 3] = this->m_tempBoardBitset[bit];
        }
        this->m_tempBoardBitset = shiftedBoardBitset;
        // Shift paths
        int shiftedPaths[ALLDIM];
        for (int i = 0; i < ALLDIM; i++) {
            shiftedPaths[i] = 0;
        }
        for (int bit = 0; bit < ALLDIM- BOARDWIDTH * 4; bit++) {
            if (this->m_tempPaths[bit] != 0) {
                shiftedPaths[bit + BOARDWIDTH * 4] = this->m_tempPaths[bit] + BOARDWIDTH * 4;
            }
        }
        for (int i = 0; i < ALLDIM; i++) {
            this->m_tempPaths[i] = shiftedPaths[i];
        }
        // Update maxRow
        this->m_tempMaxRow++;
    }

    if (col == 0) {
        // Shift board
        bitset<DIM> shiftedBoardBitset;
        for (int rr = 0; rr < BOARDWIDTH; rr++) {
            for (int cBit = 0; cBit < (BOARDWIDTH - 1) * 3; cBit++) {
                int bit = (rr * BOARDWIDTH)* 3 + cBit;
                shiftedBoardBitset[bit + 3] = this->m_tempBoardBitset[bit];
            }
        }
        this->m_tempBoardBitset = shiftedBoardBitset;
        // Shift paths
        int shiftedPaths[ALLDIM];
        for (int i = 0; i < ALLDIM; i++) {
            shiftedPaths[i] = 0;
        }
        for (int rr = 0; rr < BOARDWIDTH; rr++) {
            for (int cBit = 0; cBit < (BOARDWIDTH - 1) * 4; cBit++) {
                int bit = (rr * BOARDWIDTH) * 4 + cBit;
                if (this->m_tempPaths[bit] != 0) {
                    shiftedPaths[bit + 4] = this->m_tempPaths[bit] + 4;
                }
            }
        }
        for (int i = 0; i < ALLDIM; i++) {
            this->m_tempPaths[i] = shiftedPaths[i];
        }
        // Update maxCol
        this->m_tempMaxCol++;
    }
}

int Board::getRightEdge(int bitStart) {
    bitset<3> typeBit;
    typeBit[2] = this->m_tempBoardBitset[bitStart];
    typeBit[1] = this->m_tempBoardBitset[bitStart + 1];
    typeBit[0] = this->m_tempBoardBitset[bitStart + 2];

    switch (typeBit.to_ulong()) {
        case 1: case 6: return 2;
        case 2: case 5: return 1;
        case 3: case 4: return 0;
    }
}

void Board::printType() {
    // Output top line
    cout << "\n  ";
    for (int i = 0; i < BOARDWIDTH; i++) {
        cout << "-";
    }
    cout << "\n";
    // Output tiles
    for (int row = 0; row < BOARDWIDTH; row++) {
        cout << "| ";
        for (int col = 0; col < BOARDWIDTH; col++) {
            int bitStart = (row * BOARDWIDTH + col) * 3;
            bitset<3> type;
            type[2] = this->m_boardBitset[bitStart];
            type[1] = this->m_boardBitset[bitStart + 1];
            type[0] = this->m_boardBitset[bitStart + 2];

            switch (type.to_ulong()) {
                case 0: cout << " "; break;
                case 1: cout << "/"; break;
                case 2: cout << "+"; break;
                case 3: cout << "\\"; break;
                case 4: cout << "\\"; break;
                case 5: cout << "+"; break;
                case 6: cout << "/"; break;
            }
        }
        cout << " |\n";
    }
    // Output bottom line
    cout << "  ";
    for (int i = 0; i < BOARDWIDTH; i++) {
        cout << "-";
    }
    cout << "\n";
}

void Board::printBit() {
    for (int row = 0; row < BOARDWIDTH; row++) {
        for (int col = 0; col < BOARDWIDTH; col++) {
            int bitStart = (row * BOARDWIDTH + col) * 3;
            cout << this->m_boardBitset[bitStart] << this->m_boardBitset[bitStart + 1]
                 << this->m_boardBitset[bitStart + 2] << " ";
        }
        cout << "\n";
    }
}

TileInfo* Board::getTileInfos(bool white) {
    // Pre-handle
    int compare = white? 0: 1;
    this->m_tempBoardBitset = this->m_boardBitset; // For function getRightEdge()

    for (int row = 0; row < BOARDWIDTH; row++) {
        for (int col = 0; col < BOARDWIDTH; col++) {
            int bitStart = (row * BOARDWIDTH + col) * 3;
            // If the tile is empty, continue to next loop
            if (!(this->m_boardBitset.test(bitStart) || this->m_boardBitset.test(bitStart + 1) ||
                this->m_boardBitset.test(bitStart + 2))) {
                this->m_tileInfos[row * BOARDWIDTH + col].valid = false;
                continue;
            }

            int bit = (row * BOARDWIDTH + col) * 4;
            for (int i = 0; i < 4; i++) {
                int edge;
                if (i == 3) {
                    edge = this->m_boardBitset[bitStart + this->getRightEdge(bitStart)];
                } else {
                    edge = this->m_boardBitset[bitStart + i];
                }

                if ((this->m_paths[bit + i] != 0) && (edge == compare)) {
                    int rr = ((this->m_paths[bit + i] - 1) / 4) / BOARDWIDTH;
                    int cc = ((this->m_paths[bit + i] - 1) / 4) % BOARDWIDTH;
                    int ii = (this->m_paths[bit + i] - 1) % 4;

                    this->m_tileInfos[row * BOARDWIDTH + col].valid = true;
                    this->m_tileInfos[row * BOARDWIDTH + col].deltaRow = rr - row;
                    this->m_tileInfos[row * BOARDWIDTH + col].deltaCol = cc - col;
                    switch (ii - i) {
                        case -2:
                            this->m_tileInfos[row * BOARDWIDTH + col].angle = 2;
                            break;
                        case 3:
                            this->m_tileInfos[row * BOARDWIDTH + col].angle = -1;
                            break;
                        case -3:
                            this->m_tileInfos[row * BOARDWIDTH + col].angle = 1;
                            break;
                        default:
                            this->m_tileInfos[row * BOARDWIDTH + col].angle = ii - i;
                            break;
                    }
                    break;
                } else {
                    this->m_tileInfos[row * BOARDWIDTH + col].valid = false;
                }
            }
        }
    }
    return this->m_tileInfos;
}

bool Board::checkValid(int pos, char type) {
    /* Check if the game is end */
    if (this->m_winner != 0) {
        return false;
    }
    /* Perform single-tile-update and force play */
    this->m_tempBoardBitset = this->m_boardBitset;
    this->m_tempMaxRow = this->m_maxRow;
    this->m_tempMaxCol = this->m_maxCol;
    for (int i = 0; i < ALLDIM; i++) {
        this->m_tempPaths[i] = this->m_paths[i];
    }
    if (this->singleTileUpdate(pos, type)) {
        this->m_winner = 0;
        return true;
    }

    this->m_winner = 0;
    return false;
}

void Board::getValidPos(int pos[TILENUM][4], int* cnt) {
    *cnt = 0;
    for (int tile = 0; tile < TILENUM; tile++) {
        bool validExist = false;
        int last = 0;
        for (int tt = 1; tt <= 3; tt++) {
            char type;
            switch (tt) {
                case 1: type = '+'; break;
                case 2: type = '/'; break;
                case 3: type = '\\'; break;
            }
            if (checkValid(tile, type)) {
                if (validExist) {
                    pos[*cnt][tt] = 1;
                    last = tt + 1;
                } else {
                    pos[*cnt][0] = tile;
                    // Set invalid type to 0
                    for (int i = 1; i < tt; i++) {
                        pos[*cnt][i] = 0;
                    }
                    pos[*cnt][tt] = 1;
                    last = tt + 1;
                    validExist = true;
                }
            }
        }
        if (validExist) {
            // Set invalid type to 0
            for (int i = last; i < 4; i++) {
                pos[*cnt][i] = 0;
            }
            (*cnt)++;
        }
    }
}

void Board::getPathsFromBitset(int paths[ALLDIM]) {
    // Reset the output
    for (int i = 0; i < ALLDIM; i++) {
        paths[i] = 0;
    }

    for (int row = 0; row < BOARDWIDTH; row++) {
        for (int col = 0; col < BOARDWIDTH; col++) {
            int bitStart = (row * BOARDWIDTH + col) * 3;
            int bit = (row * BOARDWIDTH + col) * 4;
            // If the tile is empty, continue to next loop
            if (!(this->m_boardBitset.test(bitStart) || this->m_boardBitset.test(bitStart + 1) ||
                this->m_boardBitset.test(bitStart + 2))) {
                continue;
            }
            for (int i = 0; i < 4; i++) {
                // If the path is found, continue to next loop
                if (paths[bit + i] != 0) {
                    continue;
                }
                // If the neighbouring tile is not empty, continue to next loop
                switch (i) {
                    case 0:
                        if (row >= 1) {
                            int topBitStart = ((row - 1) * BOARDWIDTH + col) * 3;
                            if (this->m_boardBitset.test(topBitStart) || this->m_boardBitset.test(topBitStart + 1) ||
                                this->m_boardBitset.test(topBitStart + 2)) {
                                continue;
                            }
                        }
                        break;
                    case 1:
                        if (col >= 1) {
                            int leftBitStart = (row * BOARDWIDTH + col - 1) * 3;
                            if (this->m_boardBitset.test(leftBitStart) || this->m_boardBitset.test(leftBitStart + 1) ||
                                this->m_boardBitset.test(leftBitStart + 2)) {
                                continue;
                            }
                        }
                        break;
                    case 2:
                        if (row + 1 < BOARDWIDTH) {
                            int bottomBitStart = ((row + 1) * BOARDWIDTH + col) * 3;
                            if (this->m_boardBitset.test(bottomBitStart) || this->m_boardBitset.test(bottomBitStart + 1) ||
                                this->m_boardBitset.test(bottomBitStart + 2)) {
                                continue;
                            }
                        }
                        break;
                    case 3:
                        if (col + 1 < BOARDWIDTH) {
                            int rightBitStart = (row * BOARDWIDTH + col + 1) * 3;
                            if (this->m_boardBitset.test(rightBitStart) || this->m_boardBitset.test(rightBitStart + 1) ||
                                this->m_boardBitset.test(rightBitStart + 2)) {
                                continue;
                            }
                        }
                        break;
                }

                int nowRow = row;
                int nowCol = col;
                int nowI = i;

                bool finish = false;
                do {
                    // Find the other end in the same tile
                    bool found = false;
                    bitStart = (nowRow * BOARDWIDTH + nowCol) * 3;
                    for (int j = nowI + 1; j < 4 && found == false; j++) {
                        int endOne = (nowI == 3)? this->getRightEdge(bitStart): nowI;
                        int endTwo = (j == 3)? this->getRightEdge(bitStart): j;
                        if (this->m_boardBitset[bitStart + endOne] == this->m_boardBitset[bitStart + endTwo]) {
                            // Check neighbouring tile
                            switch (j) {
                                case 0:
                                    if (nowRow >= 1) {
                                        int topBitStart = ((nowRow - 1) * BOARDWIDTH + nowCol) * 3;
                                        if (this->m_boardBitset.test(topBitStart) || this->m_boardBitset.test(topBitStart + 1) ||
                                            this->m_boardBitset.test(topBitStart + 2)) {
                                            // Go to next tile
                                            nowRow = nowRow - 1;
                                            nowI = 2;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                                case 1:
                                    if (nowCol >= 1) {
                                        int leftBitStart = (nowRow * BOARDWIDTH + nowCol - 1) * 3;
                                        if (this->m_boardBitset.test(leftBitStart) || this->m_boardBitset.test(leftBitStart + 1) ||
                                            this->m_boardBitset.test(leftBitStart + 2)) {
                                            // Go to next tile
                                            nowCol = nowCol - 1;
                                            nowI = 3;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                                case 2:
                                    if (nowRow + 1 < BOARDWIDTH) {
                                        int bottomBitStart = ((nowRow + 1) * BOARDWIDTH + nowCol) * 3;
                                        if (this->m_boardBitset.test(bottomBitStart) || this->m_boardBitset.test(bottomBitStart + 1) ||
                                            this->m_boardBitset.test(bottomBitStart + 2)) {
                                            // Go to next tile
                                            nowRow = nowRow + 1;
                                            nowI = 0;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                                case 3:
                                    if (nowCol + 1 < BOARDWIDTH) {
                                        int rightBitStart = (nowRow * BOARDWIDTH + nowCol + 1) * 3;
                                        if (this->m_boardBitset.test(rightBitStart) || this->m_boardBitset.test(rightBitStart + 1) ||
                                            this->m_boardBitset.test(rightBitStart + 2)) {
                                            // Go to next tile
                                            nowCol = nowCol + 1;
                                            nowI = 1;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                            }
                        }
                    }
                    for (int j = 0; j < nowI && found == false; j++) {
                        int endOne = (nowI == 3)? this->getRightEdge(bitStart): nowI;
                        int endTwo = (j == 3)? this->getRightEdge(bitStart): j;
                        if (this->m_boardBitset[bitStart + endOne] == this->m_boardBitset[bitStart + endTwo]) {
                            // Check neighbouring tile
                            switch (j) {
                                case 0:
                                    if (nowRow >= 1) {
                                        int topBitStart = ((nowRow - 1) * BOARDWIDTH + nowCol) * 3;
                                        if (this->m_boardBitset.test(topBitStart) || this->m_boardBitset.test(topBitStart + 1) ||
                                            this->m_boardBitset.test(topBitStart + 2)) {
                                            // Go to next tile
                                            nowRow = nowRow - 1;
                                            nowI = j;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                                case 1:
                                    if (nowCol >= 1) {
                                        int leftBitStart = (nowRow * BOARDWIDTH + nowCol - 1) * 3;
                                        if (this->m_boardBitset.test(leftBitStart) || this->m_boardBitset.test(leftBitStart + 1) ||
                                            this->m_boardBitset.test(leftBitStart + 2)) {
                                            // Go to next tile
                                            nowCol = nowCol - 1;
                                            nowI = j;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                                case 2:
                                    if (nowRow + 1 < BOARDWIDTH) {
                                        int bottomBitStart = ((nowRow + 1) * BOARDWIDTH + nowCol) * 3;
                                        if (this->m_boardBitset.test(bottomBitStart) || this->m_boardBitset.test(bottomBitStart + 1) ||
                                            this->m_boardBitset.test(bottomBitStart + 2)) {
                                            // Go to next tile
                                            nowRow = nowRow + 1;
                                            nowI = j;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                                case 3:
                                    if (nowCol + 1 < BOARDWIDTH) {
                                        int rightBitStart = (nowRow * BOARDWIDTH + nowCol + 1) * 3;
                                        if (this->m_boardBitset.test(rightBitStart) || this->m_boardBitset.test(rightBitStart + 1) ||
                                            this->m_boardBitset.test(rightBitStart + 2)) {
                                            // Go to next tile
                                            nowCol = nowCol + 1;
                                            nowI = j;
                                            found = true;
                                        } else {
                                            // Stop and update the path
                                            int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                            paths[bit + i] = endBit + j + 1;
                                            paths[endBit + j] = bit + i + 1;
                                            found = true;
                                            finish = true;
                                        }
                                    } else {
                                        // Stop and update the path
                                        int endBit = (nowRow * BOARDWIDTH + nowCol) * 4;
                                        paths[bit + i] = endBit + j + 1;
                                        paths[endBit + j] = bit + i + 1;
                                        found = true;
                                        finish = true;
                                    }
                                    break;
                            }
                        }
                    }
                } while(!finish);
            }
        }
    }
}
