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

#define NUMTHRESHOLD 1
#define TOTALCLASS 10

/** Function prototypes **/
void playByAI(SQLiteConnection^ db, int nGame, String^ portOne, String^ portTwo);
void playWithAI(SQLiteConnection^ db, int nGame, String^ portName = "");
string checkAllWinLose(SQLiteConnection^ db, Board* board, vector<STATE>* states, int gameIndex, bool AIWhite);
void playRandom(SQLiteConnection^ db, int nGame);
string randomStep(Board* board, vector<STATE>* states, int* winner);
bool randomSteps(Board* board, vector<STATE>* states);
bool updateDb(SQLiteConnection^ db, bool win, Board board, vector<STATE> states);
string bitsetToString(STATE state);
void printStates(vector<STATE> states);
void printState(STATE state);
bool readDb(SQLiteConnection^ db, int num);
bool writeTrx(Board board, string filename, string players = "");
void saveSeperateImages(Board board, STATE state, string filenameWhite, string filenameRed);
void saveMixedImage(Board board, STATE state, string filename);
void printBoardFromBitset(string state);
void createTrainingTxt(SQLiteConnection^ db);
void createTrainingTxt_mixed(SQLiteConnection^ db);
void testCases();

int main() {
    srand(time(0));
	SQLiteConnection^ db = gcnew SQLiteConnection();
	db->ConnectionString = "Data Source = trax.db";
	db->Open();
	
	playByAI(db, 1000, "COM18", "COM24");

	//playWithAI(db, 5000, "COM18");

    //playRandom(db, 1);

    //readDb(db, 10);

	//createTrainingTxt_mixed(db); // Use it to convert the database. Please backup the database before executing it and make sure it is the original database

	db->Close();
	delete (IDisposable^)db;

	//printBoardFromBitset("000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000100010010110100000000000000000000000000000000000000000000000000011001011001000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    //testCases();

    std::cout << "Press ENTER to continue...\n";
    getchar();
    return 0;
}

void playByAI(SQLiteConnection^ db, int nGame, String^ portOne, String^ portTwo) {
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

void playWithAI(SQLiteConnection^ db, int nGame, String^ portName) {
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
			//std::cout << "Successfully record game " << game << "\n";
			// Write trx
			stringstream ss;
			ss << "./game/" << game << ".trx";
			if (writeTrx(board, ss.str(), (AIWhite? "AI vs Random": "Random vs AI"))) {
				//std::cout << "Successfully create " << game << ".trx\n";
				continue;
			}
		}
		else {
			std::cout << "Interrupte at game " << game << "\n";
			return;
		}
	}
}


string checkAllWinLose(SQLiteConnection^ db, Board* board, vector<STATE>* states, int gameIndex, bool AIWhite) {
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
					if (updateDb(db, (winner == 1) ? true : false, tempBoard, tempStates)) {
						//std::cout << "Successfully record game " << gameIndex << ", possible ending " << endIndex << "\n";
						// Write trx
						stringstream ss;
						ss << "./game/" << gameIndex << "_" << endIndex << ".trx";
						if (writeTrx(tempBoard, ss.str(), (AIWhite ? "AI vs Random" : "Random vs AI"))) {
							//std::cout << "Successfully create " << gameIndex << "_" << endIndex << ".trx\n";
						}
						endIndex++;

						record = true;
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
				/*stringstream ssWhite;
				ssWhite << "./image/W" << id << ".bmp";
				string filenameWhite = ssWhite.str();
				stringstream ssRed;
				ssRed << "./image/R" << id << ".bmp";
				string filenameRed = ssRed.str();
				saveSeperateImages(board, states[s], filenameWhite, filenameRed);*/
				stringstream ssMixed;
				ssMixed << "./image/M" << id << ".bmp";
				string filenameMixed = ssMixed.str();
				saveMixedImage(board, states[s], filenameMixed);
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

void saveSeperateImages(Board board, STATE state, string filenameWhite, string filenameRed) {
	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
	board.loadBoardFromString(bitsetToString(state));
	board.imageOutput(whiteImage, redImage);
	cv::imwrite(filenameWhite, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, whiteImage));
	cv::imwrite(filenameRed, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage));
}

void saveMixedImage(Board board, STATE state, string filename) {
	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
	board.loadBoardFromString(bitsetToString(state));
	board.imageOutput(whiteImage, redImage);
	// Create channels
	cv::Mat B = cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, whiteImage);
	cv::Mat G = cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage);
	cv::Mat R = cv::Mat::zeros(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1);
	vector<cv::Mat> channels;
	channels.push_back(B);
	channels.push_back(G);
	channels.push_back(R);
	cv::Mat img;
	cv::merge(channels, img);

	cv::imwrite(filename, img);
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
	stringstream ssSql;
	ssSql << "DELETE FROM winning_rate WHERE num < " << NUMTHRESHOLD;
	cmd->CommandText = gcnew String(ssSql.str().c_str());
	cmd->ExecuteNonQuery();
	// Calculate winning rate
	sql = "UPDATE winning_rate SET wRate = (CAST(rate AS DOUBLE) / num);";
	cmd->CommandText = gcnew String(sql.c_str());
	cmd->ExecuteNonQuery();
	// Set class
	for (int c = 0; c < TOTALCLASS; c++) {
		double min = (double)c / TOTALCLASS;
		double max = (double)(c + 1) / TOTALCLASS;

		stringstream ssSql;
		if (c == TOTALCLASS - 1) {
			max = 1.0;
			ssSql << "UPDATE winning_rate SET class = " << c << " WHERE wRate >= " << min << " AND wRate <= " << max << ";";
		}
		else {
			ssSql << "UPDATE winning_rate SET class = " << c << " WHERE wRate >= " << min << " AND wRate < " << max << ";";
		}

		cmd->CommandText = gcnew String(ssSql.str().c_str());
		cmd->ExecuteNonQuery();
	}
	/** Create TXTs for white **/
	sql = "SELECT * FROM winning_rate;";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();
	
	if (reader->HasRows) {
		ofstream fTest("./txt/testW.txt");
		ofstream fTrain("./txt/trainW.txt");
		ofstream fVal("./txt/valW.txt");
		
		int cnt = 0;
		while (reader->Read()) {
			stringstream ss;
			ss << "W" << reader->GetInt32(DBID) << ".jpeg " << reader->GetInt32(DBCLASS) << endl;
			if (cnt % 4 == 0) {
				fTest << ss.str();
			}
			else if (cnt % 5 == 0) {
				fVal << ss.str();
			} else {
				fTrain << ss.str();
			}
			// Update counter
			cnt++;
		}
		// Finish reading
		reader->Close();
		// Close files
		fTest.close();
		fTrain.close();
		fVal.close();
		std::cout << "Sucessfully create TXTs for white\n";
	}
	else {
		// Finish reading
		reader->Close();
		std::cout << "No result found\n";
	}
	/** Create TXTs for red **/
	sql = "SELECT * FROM winning_rate;";
	cmd->CommandText = gcnew String(sql.c_str());
	reader = cmd->ExecuteReader();

	if (reader->HasRows) {
		ofstream fTest("./txt/testR.txt");
		ofstream fTrain("./txt/trainR.txt");
		ofstream fVal("./txt/valR.txt");

		int cnt = 0;
		while (reader->Read()) {
			stringstream ss;
			ss << "R" << reader->GetInt32(DBID) << ".jpeg " << TOTALCLASS - 1 - reader->GetInt32(DBCLASS) << endl;
			if (cnt % 4 == 0) {
				fTest << ss.str();
			}
			else if (cnt % 5 == 0) {
				fVal << ss.str();
			}
			else {
				fTrain << ss.str();
			}
			// Update counter
			cnt++;
		}
		// Finish reading
		reader->Close();
		// Close files
		fTest.close();
		fTrain.close();
		fVal.close();
		std::cout << "Sucessfully create TXTs for red\n";
	}
	else {
		// Finish reading
		reader->Close();
		std::cout << "No result found\n";
	}
}

void createTrainingTxt_mixed(SQLiteConnection^ db) {
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
	stringstream ssSql;
	ssSql << "DELETE FROM winning_rate WHERE num < " << NUMTHRESHOLD;
	cmd->CommandText = gcnew String(ssSql.str().c_str());
	cmd->ExecuteNonQuery();
	// Calculate winning rate
	sql = "UPDATE winning_rate SET wRate = (CAST(rate AS DOUBLE) / num);";
	cmd->CommandText = gcnew String(sql.c_str());
	cmd->ExecuteNonQuery();
	// Set class
	for (int c = 0; c < TOTALCLASS; c++) {
		double min = (double)c / TOTALCLASS;
		double max = (double)(c + 1) / TOTALCLASS;

		stringstream ssSql;
		if (c == TOTALCLASS - 1) {
			max = 1.0;
			ssSql << "UPDATE winning_rate SET class = " << c << " WHERE wRate >= " << min << " AND wRate <= " << max << ";";
		}
		else {
			ssSql << "UPDATE winning_rate SET class = " << c << " WHERE wRate >= " << min << " AND wRate < " << max << ";";
		}

		cmd->CommandText = gcnew String(ssSql.str().c_str());
		cmd->ExecuteNonQuery();
	}
	/** Create TXTs for white **/
	sql = "SELECT * FROM winning_rate;";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();

	if (reader->HasRows) {
		ofstream fTest("./txt/test.txt");
		ofstream fTrain("./txt/train.txt");
		ofstream fVal("./txt/val.txt");

		int cnt = 0;
		while (reader->Read()) {
			stringstream ss;
			ss << "M" << reader->GetInt32(DBID) << ".bmp " << reader->GetInt32(DBCLASS) << endl;
			if (cnt % 4 == 0) {
				fTest << ss.str();
			}
			else if (cnt % 5 == 0) {
				fVal << ss.str();
			}
			else {
				fTrain << ss.str();
			}
			// Update counter
			cnt++;
		}
		// Finish reading
		reader->Close();
		// Close files
		fTest.close();
		fTrain.close();
		fVal.close();
		std::cout << "Sucessfully create TXTs for mixed\n";
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
    winner = board.updateBoardByCommands(string("@0/ A0/ @2+ A3+ C1+ B0+ A0/ @1+ @2/ C0/ @3/ D0+ D8/ C8/"));
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
    return;
}
