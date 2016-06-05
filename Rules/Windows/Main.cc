#include "Board.h"
#include "PortChat.cpp"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <fstream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace System;
using namespace System::Data::SQLite;

typedef bitset<DIM> STATE;
#define TRIAL 5

#define DBID 0
#define DBBOARD 1
#define DBRATE 2
#define DBNUM 3
#define DBWRATE 4
#define DBCLASS 5

/** Function prototypes **/
void playWithAI(SQLiteConnection^ db, int nGame, String^ portName = "");
void checkAllWinLose(SQLiteConnection^ db, Board* board, vector<STATE>* states, int gameIndex, bool AIWhite);
void playRandom(SQLiteConnection^ db, int nGame);
string randomStep(Board* board, vector<STATE>* states, int* winner);
bool randomSteps(Board* board, vector<STATE>* states);
bool updateDb(SQLiteConnection^ db, bool win, Board board, vector<STATE> states);
string bitsetToString(STATE state);
void printStates(vector<STATE> states);
void printState(STATE state);
bool readDb(SQLiteConnection^ db, int num);
bool writeTrx(Board board, string filename);
void saveImage(Board board, STATE state, string filenameWhite, string filenameRed);
void printBoardFromBitset(string state);
void createTrainingTxt(SQLiteConnection^ db);
void testCases();

int main() {
    srand(time(0));
	SQLiteConnection^ db = gcnew SQLiteConnection();
	db->ConnectionString = "Data Source = trax.db";
	db->Open();
	
	//playWithAI(db, 5000, "COM18");

    //playRandom(db, 1);

    //readDb(db, 10);

	createTrainingTxt(db);

	db->Close();
	delete (IDisposable^)db;

	//printBoardFromBitset("000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000100010010110100000000000000000000000000000000000000000000000000011001011001000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    //testCases();

    std::cout << "Press ENTER to continue...\n";
    getchar();
    return 0;
}

void playWithAI(SQLiteConnection^ db, int nGame, String^ portName) {
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
		if (updateDb(db, (winner == 1) ? true : false, board, states)) {
			std::cout << "Successfully record game " << game << "\n";
			// Write trx
			stringstream ss;
			ss << "./game/" << game << ".trx";
			if (writeTrx(board, ss.str())) {
				std::cout << "Successfully create " << game << ".trx\n";
				continue;
			}
		}
		else {
			std::cout << "Interrupte at game " << game << "\n";
			return;
		}
	}
}


void checkAllWinLose(SQLiteConnection^ db, Board* board, vector<STATE>* states, int gameIndex, bool AIWhite) {
	int pos[TILENUM][4];
	int posCnt;
	int choiceCnt;
	board->getValidPos(pos, &posCnt, &choiceCnt);

	int endIndex = 1;

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
					if (updateDb(db, (winner == 1) ? true : false, tempBoard, tempStates)) {
						std::cout << "Successfully record game " << gameIndex << ", possible ending " << endIndex << "\n";
						// Write trx
						stringstream ss;
						ss << "./game/" << gameIndex << "_" << endIndex << ".trx";
						if (writeTrx(tempBoard, ss.str())) {
							std::cout << "Successfully create " << gameIndex << "_" << endIndex << ".trx\n";
						}
						endIndex++;
					}
				}
			}
		}
	}
}

void playRandom(SQLiteConnection^ db, int nGame) {
    for (int game = 1; game <= nGame; game++) {
        Board board;
        vector<STATE> states;

        bool win = randomSteps(&board, &states);

        if (updateDb(db, win, board, states)) {
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

bool updateDb(SQLiteConnection^ db, bool win, Board board, vector<STATE> states) {
	SQLiteCommand^ cmd = db->CreateCommand();
	SQLiteTransaction^ tx = db->BeginTransaction();
	cmd->Transaction = tx;

	try {
		for (int s = 0; s < states.size(); s++) {
			// Check if record exists
			string sql = "SELECT * FROM winning_rate WHERE board = '" + bitsetToString(states[s]) + "';";
			cmd->CommandText = gcnew String(sql.c_str());
			SQLiteDataReader^ reader = cmd->ExecuteReader();
			if (reader->HasRows) {
				reader->Read();
				int rate = reader->GetInt32(DBRATE) + (win ? 1 : 0);
				int num = reader->GetInt32(DBNUM) + 1;
				// Finish reading
				reader->Close();

				stringstream ssRate;
				ssRate << rate;
				stringstream ssNum;
				ssNum << num;
				sql = "UPDATE winning_rate SET rate =" + ssRate.str() + ", num =" + ssNum.str() + " WHERE board = '" + bitsetToString(states[s]) + "';";
				cmd->CommandText = gcnew String(sql.c_str());
				cmd->ExecuteNonQuery();
			}
			else {
				// Finish reading
				reader->Close();
				// Insert new data
				sql = "INSERT INTO winning_rate VALUES (NULL, '" + bitsetToString(states[s]) + "'," + (win ? "1" : "0") + ",1);";
				cmd->CommandText = gcnew String(sql.c_str());
				cmd->ExecuteNonQuery();
				// Save a new image
				sql = "SELECT id FROM winning_rate WHERE board = '" + bitsetToString(states[s]) + "';";
				cmd->CommandText = gcnew String(sql.c_str());
				reader = cmd->ExecuteReader();
				reader->Read();
				int id = reader->GetInt32(0);
				reader->Close();
				stringstream ssWhite;
				ssWhite << "./image/W" << id << ".jpeg";
				string filenameWhite = ssWhite.str();
				stringstream ssRed;
				ssRed << "./image/R" << id << ".jpeg";
				string filenameRed = ssRed.str();
				saveImage(board, states[s], filenameWhite, filenameRed);
			}
		}
		tx->Commit();
		return true;
	}
	catch (SQLiteException^ E) {
		std::cout << "Fail to commit\n";
		tx->Rollback();
		return false;
	}
}

string bitsetToString(STATE state) {
    string ret = "";
    for (int i = 0; i < DIM; i++) {
        ret += state.test(i)? "1": "0";
    }
    return ret;
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

bool writeTrx(Board board, string filename) {
    ofstream ff(filename.c_str());
    if (!ff) {
        std::cout << "Cannot create " << filename << endl;
        return false;
    }
    ff << "Trax" << endl;
    ff << "White vs Red" << endl;
    vector<string> cmds = board.getCmds();
    for (int i = 0; i < cmds.size(); i ++) {
        ff << cmds[i] << " ";
    }
    ff.close();
    return true;
}

void saveImage(Board board, STATE state, string filenameWhite, string filenameRed) {
	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
	board.loadBoardFromString(bitsetToString(state));
	board.imageOutput(whiteImage, redImage);
	cv::imwrite(filenameWhite, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, whiteImage));
	cv::imwrite(filenameRed, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage));
}

void printBoardFromBitset(string state) {
	Board board;
	board.loadBoardFromString(state);
	board.printType();
}

void createTrainingTxt(SQLiteConnection^ db) {
	/** Pre-handle on database **/
	// Add column wRate
	SQLiteCommand^ cmd = db->CreateCommand();
	string sql = "ALTER TABLE winning_rate ADD COLUMN wRate DOUBLE;";
	cmd->CommandText = gcnew String(sql.c_str());
	cmd->ExecuteNonQuery();
	// Add column class
	sql = "ALTER TABLE winning_rate ADD COLUMN class integer;";
	cmd->CommandText = gcnew String(sql.c_str());
	cmd->ExecuteNonQuery();
	// Delete unneccessary data
	sql = "DELETE FROM winning_rate WHERE num < 10;";
	cmd->CommandText = gcnew String(sql.c_str());
	cmd->ExecuteNonQuery();
	// Calculate winning rate
	sql = "UPDATE winning_rate SET wRate = (CAST(rate AS DOUBLE) / num);";
	cmd->CommandText = gcnew String(sql.c_str());
	cmd->ExecuteNonQuery();
	// Set class
	for (int c = 0; c < 10; c++) {
		double min = c / 10.0;
		double max = (c + 1) / 10.0;

		stringstream ssSql;
		if (max == 1.0) {
			ssSql << "UPDATE winning_rate SET class = " << c << " WHERE wRate >= " << min << " AND wRate <= " << max << ";";
		}
		else {
			ssSql << "UPDATE winning_rate SET class = " << c << " WHERE wRate >= " << min << " AND wRate < " << max << ";";
		}

		cmd->CommandText = gcnew String(ssSql.str().c_str());
		cmd->ExecuteNonQuery();
	}
	/** Create TXTs **/
	sql = "SELECT * FROM winning_rate;";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();
	
	if (reader->HasRows) {
		ofstream fTest("./txt/test.txt");
		ofstream fTrain("./txt/train.txt");
		ofstream fVal("./txt/val.txt");
			
		while (reader->Read()) {
			// All write samples to train
			stringstream ss;
			ss << "W" << reader->GetInt32(DBID) << ".jpeg " << reader->GetInt32(DBCLASS) << endl;
			fTrain << ss.str();

			// Half of the red samples to test or val
			if (reader->GetInt32(DBID) % 2 == 0) {
				stringstream ss;
				ss << "R" << reader->GetInt32(DBID) << ".jpeg " << reader->GetInt32(DBCLASS) << endl;
				fTest << ss.str();
			}
			else {
				stringstream ss;
				ss << "R" << reader->GetInt32(DBID) << ".jpeg " << reader->GetInt32(DBCLASS) << endl;
				fVal << ss.str();
			}
		}
		// Finish reading
		reader->Close();
		// Close files
		fTest.close();
		fTrain.close();
		fVal.close();
		std::cout << "Sucessfully create TXTs\n";
	}
	else {
		// Finish reading
		reader->Close();
		std::cout << "No result found\n";
	}
}

void testCases() {
    /** Testing for Board **/
    Board board;
    // Test update and print
    int winner;
    winner = board.updateBoardByCommands(string("@0/ A0/ A3+ A4\\ B3/ B1+ A0/ @2/ A5\\ D2/ E1\\ B6+ @6/"));
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
    winner = board.updateBoardByCommands(string("@0/ A2\\ A3+ B3+ C3/ C2/"));
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
	for (int row = 0; row < OUTPUTWIDTH; row++) {
		for (int col = 0; col < OUTPUTWIDTH; col++) {
			std::cout << white[row * OUTPUTWIDTH + col] << " ";
		}
		std::cout << endl;
	}
	std::cout << endl;
	/*
	for (int row = 0; row < OUTPUTWIDTH; row++) {
		for (int col = 0; col < OUTPUTWIDTH; col++) {
			std::cout << red[row * OUTPUTWIDTH + col] << " ";
		}
		std::cout << endl;
	}
	*/
	// Test image output
	board.loadBoardFromString("000000000000000000000000000000000000000000000000000000000000000100010010000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
	board.printType();
	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
	board.imageOutput(whiteImage, redImage);
	cv::imwrite("test.jpeg", cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage));
    return;
}
