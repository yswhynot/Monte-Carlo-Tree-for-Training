#include "Board.h"
#include "PortChat.cpp"
#include "Database.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TRIAL 300

/** Function prototypes **/
void playByAI(Database^ db, int nGame, String^ portOne, String^ portTwo);
void playWithAI(Database^ db, int nGame, String^ portName = "");
string checkAllWinLose(Database^ db, Board* board, vector<STATE>* states, int gameIndex, bool AIWhite);
void playRandom(Database^ db, int nGame);
string randomStep(Board* board, vector<STATE>* states, int* winner);
bool randomSteps(Board* board, vector<STATE>* states);
void printStates(vector<STATE> states);
void printState(STATE state);
bool readDb(Database^ db, int num);
bool writeTrx(Board board, string filename, string players = "");
void printBoardFromBitset(string state);
void testCases();

int main() {
    srand(time(0));
	
	Database^ db = gcnew Database();
	
	//playByAI(db, 1000, "COM18", "COM24");

	//playWithAI(db, 5000, "COM18");

	//playRandom(db, 1);

	//readDb(db, 10);

	//db->createVariations();

	//db->saveFeatureMaps();

	//db->createTrainingTxt(true);

    testCases();

    std::cout << "Press ENTER to continue...\n";
    getchar();
    return 0;
}

void playByAI(Database^ db, int nGame, String^ portOne, String^ portTwo) {
	PortChat chatOne(portOne);
	PortChat chatTwo(portTwo);

	String^ msg;
	bool chatOneWhite = false;
	bool lastPortOne = false;
	for (int game = 1; game <= nGame; game++) {
		Board board;
		vector<STATE> states;
		int winner;

		// Receive from port one
		if (game == 1) {
			bool hasRead = false;
			do {
				std::cout << "Waiting for port one...\n";
				msg = chatOne.Read(&hasRead);
			} while (hasRead == false);
			// Allocate color for port one
			chatOneWhite = (msg->Equals("-B"));
			// Check if port two has the right color
			hasRead = false;
			do {
				std::cout << "Waiting for port two...\n";
				msg = chatTwo.Read(&hasRead);
			} while (hasRead == false);
			if (msg->Equals((chatOneWhite ? "-W" : "-B")) == false) {
				std::cout << "Port two color invalid, interrupte at game " << game << "\n";
				return;
			}
		}
		else {
			if (lastPortOne) {
				// Allocate color for port one
				chatOneWhite = (msg->Equals("-B"));
				// Check if port two has the right color
				bool hasRead = false;
				do {
					msg = chatTwo.Read(&hasRead);
				} while (hasRead == false);
				if (msg->Equals((chatOneWhite ? "-W" : "-B")) == false) {
					std::cout << "Port two color invalid, interrupte at game " << game << "\n";
					return;
				}
			}
			else {
				// Use color from port two to set the color of port one
				chatOneWhite = (msg->Equals("-W"));
				// Check if port one has the right color
				bool hasRead = false;
				do {
					msg = chatOne.Read(&hasRead);
				} while (hasRead == false);
				if (msg->Equals((chatOneWhite ? "-B" : "-W")) == false) {
					std::cout << "Port one color invalid, interrupte at game " << game << "\n";
					return;
				}
			}
		}
		// If port two is white, handle it first
		if (chatOneWhite == false) {
			bool hasRead = false;
			int trialCnt = 0;
			do {
				msg = chatTwo.Read(&hasRead);
				// Trial update
				trialCnt++;
				if (trialCnt >= TRIAL) {
					std::cout << "Interrupte at game " << game << "\n";
					return;
				}
			} while (hasRead == false);
			// Update board
			char* msgChr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(msg);
			board.updateBoardByCommand(string(msgChr), &winner);
			states.push_back(board.getBoardBitset());
			// Send to port one
			chatOne.Write(msg);
		}
		/** Ordinary procedures **/
		do {
			// Listen from port one
			bool hasRead = false;
			int trialCnt = 0;
			do {
				msg = chatOne.Read(&hasRead);
				// Trial update
				trialCnt++;
				if (trialCnt >= TRIAL) {
					std::cout << "Interrupte at game " << game << "\n";
					return;
				}
			} while (hasRead == false);
			// Check if port one win the game
			if (msg->Equals("-W") || msg->Equals("-B")) {
				String^ cmd = gcnew String(checkAllWinLose(db, &board, &states, game, chatOneWhite).c_str());
				// Send to port two
				chatTwo.Write(cmd);

				lastPortOne = true;
				break; /* do */
			}
			// Else, update board
			char* msgChr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(msg);
			board.updateBoardByCommand(string(msgChr), &winner);
			states.push_back(board.getBoardBitset());
			// Send to port two
			chatTwo.Write(msg);
			// Listen from port two
			hasRead = false;
			trialCnt = 0;
			do {
				msg = chatTwo.Read(&hasRead);
				// Trial update
				trialCnt++;
				if (trialCnt >= TRIAL) {
					std::cout << "Interrupte at game " << game << "\n";
					return;
				}
			} while (hasRead == false);
			// Check if port two win the game
			if (msg->Equals("-W") || msg->Equals("-B")) {
				String^ cmd = gcnew String(checkAllWinLose(db, &board, &states, game, !chatOneWhite).c_str());
				// Send to port one
				chatOne.Write(cmd);

				lastPortOne = false;
				break; /* do */
			}
			// Else, update board
			msgChr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(msg);
			board.updateBoardByCommand(string(msgChr), &winner);
			states.push_back(board.getBoardBitset());
			// Send to port one
			chatOne.Write(msg);
		} while (true);
	}
}

void playWithAI(Database^ db, int nGame, String^ portName) {
	std::cout << "Start playing...\n";

	PortChat portChat(portName);

	String^ msg;
	bool AIWhite = true; // The AI color is different form the command -W (false) and -B (true)
	bool waitForInit = false;
	for (int game = 1; game <= nGame; game++) {
		Board board;
		vector<STATE> states;
		int winner;
		
		/** Handle initial message for color **/
		if (game == 1 || waitForInit) {
			// Listen to port
			bool hasRead = false;
			do {
				if (!waitForInit) std::cout << "Waiting for initial command...\n";
				msg = portChat.Read(&hasRead);
			} while (hasRead == false);
			// Clear flag
			waitForInit = false;
		}
		// If white, do the first step
		AIWhite = true;
		if (msg->Equals("-W")) {
			string cmd = randomStep(&board, &states, &winner);
			portChat.Write(gcnew String(cmd.c_str()));
			// Update AI color
			AIWhite = false;
		}
		else if (!msg->Equals("-B")) {
			std::cout << "Interrupte at game " << game << "\n";
			return;
		}
		/** Ordinary Procedures **/
		bool handle = false;
		do {
			// Listen to the command
			bool hasRead = false;
			int trialCnt = 0;
			do {
				msg = portChat.Read(&hasRead);
				// Trial update
				trialCnt++;
				if (trialCnt >= TRIAL) {
					std::cout << "Interrupte at game " << game << "\n";
					return;
				}
			} while (hasRead == false);
			// If the last game is end, interrupt
			if (msg->Equals("-W") || msg->Equals("-B")) {
				checkAllWinLose(db, &board, &states, game, AIWhite);
				handle = true;
				// Update AI color
				AIWhite = !msg->Equals("-W");
				break; /* do */
			}
			// Else, game not end
			char* msgChr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(msg);
			board.updateBoardByCommand(string(msgChr), &winner);
			states.push_back(board.getBoardBitset());
			// Make response
			string cmd = randomStep(&board, &states, &winner);
			portChat.Write(gcnew String(cmd.c_str()));

			if (winner != 0) {
				waitForInit = true;
				break;
			}
		} while (winner == 0);

		// If the game is end, continue
		if (handle) continue;

		/** Update database **/
		if (db->updateDb((winner == 1) ? true : false, board, states, ALLVARIATION)) {
			// Write trx
			stringstream ss;
			ss << "./game/" << game << ".trx";
			writeTrx(board, ss.str(), (AIWhite ? "AI vs Random" : "Random vs AI"));
			// Output
			if (game % 100 == 0) {
				std::cout << "Successfully created " << game << " games\n";
			}
			continue;
		}
		else {
			std::cout << "Interrupte at game " << game << "\n";
			return;
		}
	}
}


string checkAllWinLose(Database^ db, Board* board, vector<STATE>* states, int gameIndex, bool AIWhite) {
	int pos[TILENUM][4];
	int posCnt;
	int choiceCnt;
	board->getValidPos(pos, &posCnt, &choiceCnt);

	int endIndex = 1;

	string ret; // Return one of the last step of winning game

	bool record = false;

	for (int pp = 0; pp < posCnt; pp++) {
		for (int cc = 1; cc <= 3; cc++) {
			if (pos[pp][cc] == 1) {
				char type;
				switch (cc) {
				case 1: type = '+'; break;
				case 2: type = '/'; break;
				case 3: type = '\\'; break;
				}
				// Backup board and states
				Board tempBoard = *board;
				int winner;
				tempBoard.updateBoard(pos[pp][0], type, &winner);
				if (winner == (AIWhite? 1: 2)) {
					vector<STATE> tempStates = *states;
					tempStates.push_back(tempBoard.getBoardBitset());
					if (db->updateDb((winner == 1) ? true : false, tempBoard, tempStates, ALLVARIATION)) {
						// Write trx
						stringstream ss;
						ss << "./game/" << gameIndex << "_" << endIndex << ".trx";
						writeTrx(tempBoard, ss.str(), (AIWhite ? "AI vs Random" : "Random vs AI"));
						endIndex++;
						// Update record flag
						record = true;
						// Output
						if (gameIndex % 100 == 0) {
							std::cout << "Successfully created " << gameIndex << " games\n";
						}
					}
					// Update ret
					ret = tempBoard.getCmds().back();
				}
			}
		}
	}

	if (record == false) {
		std::cout << "No possible ending for game " << gameIndex << endl;
		// Write trx
		stringstream ss;
		ss << "./game/" << gameIndex << ".trx";
		writeTrx(*board, ss.str(), (AIWhite ? "AI vs Random" : "Random vs AI"));
	}

	return ret;
}

void playRandom(Database^ db, int nGame) {
    for (int game = 1; game <= nGame; game++) {
        Board board;
        vector<STATE> states;

        bool win = randomSteps(&board, &states);

        if (db->updateDb(win, board, states, ALLVARIATION)) {
            std::cout << "Successfully record game " << game << "\n";
            // Write trx
            stringstream ss;
            ss << "./game/" << game << ".trx";
            if (writeTrx(board, ss.str())) {
                std::cout << "Successfully create " << game << ".trx\n";
            }
        } else {
            std::cout << "Interrupte at game " << game << "\n";
            break;
        }

        //printStates(states);
    }
}

string randomStep(Board* board, vector<STATE>* states, int* winner) {
	int pos[TILENUM][4];
	int posCnt;
	int choiceCnt;
	board->getValidPos(pos, &posCnt, &choiceCnt);
	int choose = rand() % choiceCnt;

	int cCnt = 0;
	bool next = true;
	for (int pp = 0; pp < posCnt && next; pp++) {
		for (int cc = 1; cc <= 3 && next; cc++) {
			if (pos[pp][cc] == 1) {
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
	states->push_back(board->getBoardBitset());
	return board->getCmds().back();
}

bool randomSteps(Board* board, vector<STATE>* states) {
    int winner;
    do {
		randomStep(board, states, &winner);
    } while (winner == 0);
    return (winner == 1)? true: false;
}

void printStates(vector<STATE> states) {
    for (int s = 0; s < (states.size()>=10? 10: states.size()); s++) {
        printState(states[s]);
    }
}

void printState(STATE state) {
    // Output top line
    std::cout << "\n  ";
    for (int i = 0; i < BOARDWIDTH; i++) {
        std::cout << "-";
    }
    std::cout << "\n";
    // Output tiles
    for (int row = 0; row < BOARDWIDTH; row++) {
        std::cout << "| ";
        for (int col = 0; col < BOARDWIDTH; col++) {
            int bitStart = (row * BOARDWIDTH + col) * 3;
            bitset<3> type;
            type[2] = state[bitStart];
            type[1] = state[bitStart + 1];
            type[0] = state[bitStart + 2];

            switch (type.to_ulong()) {
                case 0: std::cout << " "; break;
                case 1: std::cout << "/"; break;
                case 2: std::cout << "+"; break;
                case 3: std::cout << "\\"; break;
                case 4: std::cout << "\\"; break;
                case 5: std::cout << "+"; break;
                case 6: std::cout << "/"; break;
            }
        }
        std::cout << " |\n";
    }
    // Output bottom line
    std::cout << "  ";
    for (int i = 0; i < BOARDWIDTH; i++) {
        std::cout << "-";
    }
    std::cout << "\n";
}

bool readDb(SQLiteConnection^ db, int num) {
	SQLiteCommand^ cmd = db->CreateCommand();
    string sql = "SELECT * FROM winning_rate ORDER BY num DESC, rate DESC;";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();

	if (reader->HasRows) {
		int cnt = 1;
		while (reader->Read() && cnt <= num) {
			std::cout << "Pattern " << cnt << ":\n";
			// Handle bitset
			char* sStr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(reader->GetString(DBBOARD));
			STATE s;
			for (int bit = 0; bit < DIM; bit++) {
				if ((sStr[bit] - '0') == 0) {
					s.reset(bit);
				}
				else {
					s.set(bit);
				}
			}
			printState(s);
			// Handle data
			int rate = reader->GetInt32(DBRATE);
			int rNum = reader->GetInt32(DBNUM);
			double wRate = (double)rate / cnt;
			std::cout << "Winning number: " << rate << ", total number: " << rNum << "\n";
			printf("Winning rate: %.2f\n", wRate);
			// Update counter
			cnt++;
		}
		// Finish reading
		reader->Close();
	}
	else {
		// Finish reading
		reader->Close();
		std::cout << "No result found\n";
	}
	return true;
}

bool writeTrx(Board board, string filename, string players) {
    ofstream ff(filename.c_str());
    if (!ff) {
        std::cout << "Cannot create " << filename << endl;
        return false;
    }
    ff << "Trax" << endl;
    ff << ((players == "")? "White vs Red": players) << endl;
    vector<string> cmds = board.getCmds();
    for (int i = 0; i < cmds.size(); i ++) {
        ff << cmds[i] << " ";
    }
    ff.close();
    return true;
}

void printBoardFromBitset(string state) {
	Board board;
	board.loadBoardFromString(state);
	board.printType();
}

void testCases() {
    /** Testing for Board **/
    Board board;
    // Test update and print
    int winner;
    winner = board.updateBoardByCommands(string("@0+ B1/ A0\\ B3/ C1\\ D2+ B4/ @3\\ B5+ D0/ E2+ F2/ E1/ E0/ F1/ A1\\ @4/ B0/"));
    board.printType();
    std::cout << "Player "<< winner << " wins the game.\n";
	// Test command list
	vector<string> cmds = board.getCmds();
	for (int t = 0; t < cmds.size(); t++) {
		std::cout << cmds[t] << " ";
	}
	std::cout << endl;
    // Test check valid positions
    board.reset();
    winner = board.updateBoardByCommands(string("@0/ A2\\ B2\\ B3+ B4/ C4+ B5+ @3\\ B5+ B0/ B0+ @6+"));
    board.printType();
    int pos[TILENUM][4];
    int posCnt;
    int choiceCnt;
    board.getValidPos(pos, &posCnt, &choiceCnt);
    for (int i = 0; i < posCnt; i++) {
        std::cout << pos[i][0] << ": " << (pos[i][1] == 1? "+": "") << (pos[i][2] == 1? "/": "") << (pos[i][3] == 1? "\\": "") << "\n";
    }
    std::cout << "Number of possible choices: " << choiceCnt << "\n";
    std::cout << "\n";
    // Test output TileInfo
    TileInfo* tileInfos;
    tileInfos = board.getTileInfos(true);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
			std::cout << i << ": " << ((tileInfos[i].endPoint) ? " End Point" : " ") << ((tileInfos[i].attack) ? " Attack":"") << "\n";
        }
    }
    std::cout << "\n";
    tileInfos = board.getTileInfos(false);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
			std::cout << i << ": " << ((tileInfos[i].endPoint) ? " End Point" : " ") << ((tileInfos[i].attack) ? " Attack":"") << "\n";
        }
    }
    // Test getPathsFromBitset()
    int paths[ALLDIM];
    board.getPathsFromBitset(paths);
	// Test board converter
	board.reset();
	board.updateBoardByCommands(string("@0/ @1\\"));
	board.printType();
	bool white[OUTPUTWIDTH * OUTPUTWIDTH];
	bool red[OUTPUTWIDTH * OUTPUTWIDTH];
	board.boardConverter(white, red);
	/*
	for (int row = 0; row < OUTPUTWIDTH; row++) {
		for (int col = 0; col < OUTPUTWIDTH; col++) {
			std::cout << white[row * OUTPUTWIDTH + col] << " ";
		}
		std::cout << endl;
	}
	std::cout << endl;
	*/
	/*
	for (int row = 0; row < OUTPUTWIDTH; row++) {
		for (int col = 0; col < OUTPUTWIDTH; col++) {
			std::cout << red[row * OUTPUTWIDTH + col] << " ";
		}
		std::cout << endl;
	}
	*/
	// Test image output
	board.loadBoardFromString("000000000000000000000000000000000000000000000000000000000000000001110100000000000000000000000000000000000000000000000000000110001011000000000000000000000000000000000000000000000000000000110100000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
	board.printType();
	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
	board.imageOutput(whiteImage, redImage);
	cv::Mat B = cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, whiteImage);
	cv::Mat G = cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage);
	cv::Mat R = cv::Mat::zeros(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1);
	vector<cv::Mat> channels;
	channels.push_back(B);
	channels.push_back(G);
	channels.push_back(R);
	cv::Mat img;
	cv::merge(channels, img);
	cv::imwrite("white.bmp", cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, whiteImage));
	cv::imwrite("red.bmp", cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage));
	cv::imwrite("mix.bmp", img);
	// Test clockwise and flip
	board.clockwise();
	board.printType();
	board.flip();
	board.printType();
    return;
}
