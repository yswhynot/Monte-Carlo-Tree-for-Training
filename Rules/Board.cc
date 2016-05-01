#include "Board.h"

#include <iostream>
using namespace std;

Board::Board() {
    this->start = true;
}

bool Board::updateBoard(int pos, char type) {
    /* Backup current board */
    this->m_tempBoardBitset = this->m_boardBitset;
    /* Perform single-tile-update and force play */
    if (this->singleTileUpdate(pos, type)) {
        this->m_boardBitset = this->m_tempBoardBitset;
        return true;
    } else {
        return false;
    }
}

bool Board::singleTileUpdate(int pos, char type) {
    int bitStart = pos * 3;
    /* If the tile is the first one */
    if (this->start) {

        switch (type) {
            case '+':
                this->m_tempBoardBitset[bitStart] = 0;
                this->m_tempBoardBitset[bitStart + 1] = 1;
                this->m_tempBoardBitset[bitStart + 2] = 0;
                break;
            case '/':
                this->m_tempBoardBitset[bitStart] = 1;
                this->m_tempBoardBitset[bitStart + 1] = 1;
                this->m_tempBoardBitset[bitStart + 2] = 0;
                break;
        }
        this->start = false;
        return true;
    }
    /* Check if current tile is not empty */
    if (this->m_tempBoardBitset.test(bitStart) || this->m_tempBoardBitset.test(bitStart + 1) ||
        this->m_tempBoardBitset.test(bitStart + 2)) {
        return false;
    }
    /* Get four neighboring edges */
    int row = pos / BOARDWIDTH;
    int col = pos % BOARDWIDTH;

    int topPos = (pos - BOARDWIDTH) * 3;
    int leftPos = (pos - 1) * 3;
    int bottomPos = (pos + BOARDWIDTH) * 3;
    int rightPos = (pos + 1) * 3;

    bitset<4> whiteBits, redBits;

    if (row >= 1) {
        if (this->m_tempBoardBitset.test(topPos) || this->m_tempBoardBitset.test(topPos + 1) ||
            this->m_tempBoardBitset.test(topPos + 2)) {
            whiteBits[0] = 1 - this->m_tempBoardBitset[topPos + 2];
            redBits[0] = this->m_tempBoardBitset[topPos + 2];
        }
    }

    if (col >= 1) {
        if (this->m_tempBoardBitset.test(leftPos) || this->m_tempBoardBitset.test(leftPos + 1) ||
            this->m_tempBoardBitset.test(leftPos + 2)) {
            whiteBits[1] = 1 - this->m_tempBoardBitset[leftPos + getRightEdge(leftPos)];
            redBits[1] = this->m_tempBoardBitset[leftPos + getRightEdge(leftPos)];
        }
    }

    if (row + 1 < BOARDWIDTH) {
        if (this->m_tempBoardBitset.test(bottomPos) || this->m_tempBoardBitset.test(bottomPos + 1) ||
            this->m_tempBoardBitset.test(bottomPos + 2)) {
            whiteBits[2] = 1 - this->m_tempBoardBitset[bottomPos];
            redBits[2] = this->m_tempBoardBitset[bottomPos];
        }
    }

    if (col + 1 < BOARDWIDTH) {
        if (this->m_tempBoardBitset.test(rightPos) || this->m_tempBoardBitset.test(rightPos + 1) ||
            this->m_tempBoardBitset.test(rightPos + 2)) {
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
            }
            break;
        default: return false;
    }
    // Check force play
    if (this->forcePlay(pos)) {
        return true;
    } else {
        return false;
    }
}

bool Board::forcePlay(int pos) {
    int row = pos / BOARDWIDTH;
    int col = pos % BOARDWIDTH;
    // Check top
    if (row >= 2) {
        int bitStart = ((row - 2) * BOARDWIDTH + col) * 3;
        if ((this->m_tempBoardBitset[bitStart] != 0) && (this->m_tempBoardBitset[bitStart + 1] != 0) &&
            (this->m_tempBoardBitset[bitStart + 2] != 0)) { // The tilt to check is not empty
            // Bottom of the tilt
            if (this->m_tempBoardBitset[pos * 3] == this->m_tempBoardBitset[bitStart + 2]) {
                if (singleTileUpdate((row - 1) * BOARDWIDTH + col, '+') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3] == this->m_tempBoardBitset[bitStart + this->getRightEdge(bitStart)]) {
                if (singleTileUpdate((row - 1) * BOARDWIDTH + col, '\\') == false) {
                    return false;
                }
            }

            // Bottom of the tilt
            if (this->m_tempBoardBitset[pos * 3 + 1] == this->m_tempBoardBitset[bitStart + 2]) {
                if (singleTileUpdate(row * BOARDWIDTH + col - 1, '\\') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3 + 1] == this->m_tempBoardBitset[bitStart + this->getRightEdge(bitStart)]) {
                if (singleTileUpdate(row * BOARDWIDTH + col - 1, '+') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3 + 1] == this->m_tempBoardBitset[bitStart]) {
                if (singleTileUpdate(row * BOARDWIDTH + col - 1, '/') == false) {
                    return false;
                }
            }

            // Right of the tilt
            if (this->m_tempBoardBitset[pos * 3 + 2] == this->m_tempBoardBitset[bitStart + this->getRightEdge(bitStart)]) {
                if (singleTileUpdate((row + 1) * BOARDWIDTH + col, '/') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3 + 2] == this->m_tempBoardBitset[bitStart]) {
                if (singleTileUpdate((row + 1) * BOARDWIDTH + col, '+') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3 + 2] == this->m_tempBoardBitset[bitStart + 1]) {
                if (singleTileUpdate((row + 1) * BOARDWIDTH + col, '\\') == false) {
                    return false;
                }
            }

            // Top of the tilt
            if (this->m_tempBoardBitset[pos * 3 + this->getRightEdge(pos * 3)] == this->m_tempBoardBitset[bitStart]) {
                if (singleTileUpdate(row * BOARDWIDTH + col + 1, '\\') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3 + this->getRightEdge(pos * 3)] == this->m_tempBoardBitset[bitStart + 1]) {
                if (singleTileUpdate(row * BOARDWIDTH + col + 1, '+') == false) {
                    return false;
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
            if (this->m_tempBoardBitset[pos * 3 + this->getRightEdge(pos * 3)] == this->m_tempBoardBitset[bitStart + 2]) {
                if (singleTileUpdate(row * BOARDWIDTH + col + 1, '/') == false) {
                    return false;
                }
            }

            // Left of the tilt
            if (this->m_tempBoardBitset[pos * 3] == this->m_tempBoardBitset[bitStart + 1]) {
                if (singleTileUpdate((row - 1) * BOARDWIDTH + col, '/') == false) {
                    return false;
                }
            }
        }
    }
    return true;
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

void Board::print() {
    for (int row = 0; row < BOARDWIDTH; row++) {
        for (int col = 0; col < BOARDWIDTH; col++) {
            int bitStart = ((row * BOARDWIDTH) + col) * 3;
            bitset<3> type;
            type[2] = this->m_tempBoardBitset[bitStart];
            type[1] = this->m_tempBoardBitset[bitStart + 1];
            type[0] = this->m_tempBoardBitset[bitStart + 2];

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
        cout << "\n";
    }
}
