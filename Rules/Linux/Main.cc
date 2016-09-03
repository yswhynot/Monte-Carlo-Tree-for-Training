#include "Board.h"
#include "PortChat.cpp"

#include <time.h>

#include <opencv.hpp>

typedef bitset<DIM> STATE;

/** Function prototypes **/
string nextStep(Board* board, vector<STATE>* states, int* winner);
void saveTestingImage(Board board);

int main() {
    srand(time(0));

    /** BEGIN **/
    int nGame = 1;
    PortChat portChat("/dev/ttyUSB0");

    std::cout << "Start playing...\n";

	string msg;
	bool waitForInit = false;
	for (int game = 1; game <= nGame; game++) {
		Board board;
		vector<STATE> states;
		int winner;

		/** Handle initial message for color **/
		if (game == 1 || waitForInit) {
            if (!waitForInit) std::cout << "Waiting for initial command...\n";
			// Listen to port
			bool hasRead = false;
			do {
				msg = portChat.Read(&hasRead, 3);
			} while (!hasRead);
			// Clear flag
			waitForInit = false;
		}
		// If white, do the first step
		if (msg.compare("-W") == 0) {
			string cmd = nextStep(&board, &states, &winner);
			portChat.Write(cmd);
		}
		else if (msg.compare("-B") != 0) {
			std::cout << "Interrupte at game " << game << "\n";
			return -1;
		}
		/** Ordinary Procedures **/
		do {
			// Listen to the command
			bool hasRead = false;
			do {
				msg = portChat.Read(&hasRead, 4);
			} while (hasRead == false);
			// If the last game is end, interrupt
			if (msg.compare("-W") == 0 || msg.compare("-B") == 0) {
				break; /* do */
			}
			// Else, game not end
			board.updateBoardByCommand(msg, &winner);
			states.push_back(board.getBoardBitset());
			// Make response
			string cmd = nextStep(&board, &states, &winner);
			portChat.Write(cmd);

			if (winner != 0) {
				waitForInit = true;
				break;
			}
		} while (winner == 0);
	}
    /** END **/

    std::cout << "Press ENTER to continue...\n";
    getchar();
    return 0;
}

string nextStep(Board* board, vector<STATE>* states, int* winner) {
	// Get all possible position
	int pos[TILENUM][4];
	int posCnt;
	int choiceCnt;
	board->getValidPos(pos, &posCnt, &choiceCnt);

    // Create memory to save win rate
    int winRate[TILENUM][4];
    for (int pp = 0; pp < posCnt; pp++) {
        winRate[pp][0] = pos[pp][0];
    }

    int maxRate = 0;
	for (int pp = 0; pp < posCnt; pp++) {
		for (int cc = 1; cc <= 3; cc++) {
			if (pos[pp][cc] == 1) {
                char type;
                switch (cc) {
                case 1: type = '+'; break;
                case 2: type = '/'; break;
                case 3: type = '\\'; break;
                }

                // Try possible position
                Board tempBoard = *board;
                tempBoard.updateBoard(pos[pp][0], type, winner);

                // Save the map
                saveTestingImage(tempBoard);

                // use caffe to calculate the win rate
                int rate = 0;

                // Save the possibility
                winRate[pp][cc] = rate;
                if (rate > maxRate) {
                    maxRate = rate;
                }
			}
		}
	}

	// Find number of maximum rate
	int maxRateCnt = 0;
	for (int pp = 0; pp < posCnt; pp++) {
		for (int cc = 1; cc <= 3; cc++) {
			if (pos[pp][cc] == 1) {
                if (winRate[pp][cc] == maxRate) {
                    maxRateCnt++;
                }
            }
		}
	}

	// Randomly choose one maximum rate position
	int choose = rand() % maxRateCnt;
	int cCnt = 0;
	bool next = true;
	for (int pp = 0; pp < posCnt && next; pp++) {
		for (int cc = 1; cc <= 3 && next; cc++) {
			if (pos[pp][cc] == 1) {
                if (winRate[pp][cc] == maxRate) {
                    if (cCnt == choose) {
                        char type;
                        switch (cc) {
                        case 1: type = '+'; break;
                        case 2: type = '/'; break;
                        case 3: type = '\\'; break;
                        }
                        board->updateBoard(pos[pp][0], type, winner);
                        next = false;
                    }
                    else {
                        cCnt++;
                    }
                }
			}
		}
	}

	states->push_back(board->getBoardBitset());
	return board->getCmds().back();
}

void saveTestingImage(Board board) {
	unsigned char map[OUTPUTWIDTH * OUTPUTWIDTH];
	board.mapOutput(map);
	cv::imwrite("test.bmp", cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, map));
}
