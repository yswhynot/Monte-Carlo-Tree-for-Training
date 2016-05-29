#include "Board.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <fstream>

#include "PortChat.cpp"

using namespace std;
using namespace System;
using namespace System::Data::SQLite;

typedef bitset<DIM> STATE;

/** Function prototypes **/
void playGames(SQLiteConnection^ db, int nGame);
bool randomSteps(Board* board, vector<STATE>* states);
bool updateDb(SQLiteConnection^ db, bool win, vector<STATE> states);
string bitsetToString(STATE state);
void printStates(vector<STATE> states);
void printState(STATE state);
bool readDb(SQLiteConnection^ db, int num);
bool writeTrx(Board board, string filename);
void testCases();

int main() {
    srand(time(0));
	SQLiteConnection^ db = gcnew SQLiteConnection();
	db->ConnectionString = "Data Source = trax.db";
	db->Open();

    //playGames(db, 100);

    readDb(db, 10);

	db->Close();
	delete (IDisposable^)db;

	PortChat::Main();

    //testCases();

    std::cout << "Press ENTER to continue...\n";
    getchar();
    return 0;
}

void playGames(SQLiteConnection^ db, int nGame) {
    for (int game = 1; game <= nGame; game++) {
        Board board;
        vector<STATE> states;

        bool win = randomSteps(&board, &states);

        if (updateDb(db, win, states)) {
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

bool randomSteps(Board* board, vector<STATE>* states) {
    int winner;
    do {
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
                        board->updateBoard(pos[pp][0], type, &winner);
                        next = false;
                    } else {
                        cCnt++;
                    }
                }
            }
        }
        states->push_back(board->getBoardBitset());
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

bool updateDb(SQLiteConnection^ db, bool win, vector<STATE> states) {
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
				int rate = reader->GetInt32(1) + (win ? 1 : 0);
				int num = reader->GetInt32(2) + 1;
				stringstream ssRate;
				ssRate << rate;
				stringstream ssNum;
				ssNum << num;
				sql = "UPDATE winning_rate SET rate =" + ssRate.str() + ", num =" + ssNum.str() + " WHERE board = '" + bitsetToString(states[s]) + "';";
				cmd->CommandText = gcnew String(sql.c_str());
				cmd->ExecuteNonQuery();
			}
			else {
				// Insert new data
				sql = "INSERT INTO winning_rate VALUES ('" + bitsetToString(states[s]) + "'," + (win ? "1" : "0") + ",1);";
				cmd->CommandText = gcnew String(sql.c_str());
				cmd->ExecuteNonQuery();
			}
			String^ x = reader->GetString(0);
			int a = 0;
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
			char* sStr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(reader->GetString(0));
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
			int rate = reader->GetInt32(1);
			int rNum = reader->GetInt32(2);
			double wRate = (double)rate / cnt;
			std::cout << "Winning number: " << rate << ", total number: " << rNum << "\n";
			printf("Winning rate: %.2f\n", wRate);
			// Update counter
			cnt++;
		}
	}
	else {
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
    ff << "Write vs Red" << endl;
    vector<string> cmds = board.getCmds();
    for (int i = 0; i < cmds.size(); i ++) {
        ff << cmds[i] << " ";
    }
    ff.close();
    return true;
}

void testCases() {
    /** Testing for Board **/
    Board board;
    // Test update and print
    int winner;
    winner = board.updateBoardByCommands(string("@0+ B1\\ B0+ B0/ @3\\ A2\\ C4+ D1/ D0+ B6/ B0\\ D1+ C7+ A7/ E1+ C0\\ @4\\ C9+ C10\\ A6\\ E8+ C1+ D0\\ C0\\ D1/ D0+ B13+ @9/ B6\\ F1+ E0/ F0\\ E16+ G5\\ D3\\ G3+ C1\\ G13/ C4/"));
    board.printType();
    std::cout << "Player "<< winner << " wins the game.\n";
    // Test check valid positions
    board.reset();
    winner = board.updateBoardByCommands(string("@0+ A0\\ A3/"));
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
            std::cout << i << ": " << tileInfos[i].deltaRow << " " << tileInfos[i].deltaCol << " " << tileInfos[i].angle << "\n";
        }
    }
    std::cout << "\n";
    tileInfos = board.getTileInfos(false);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
            std::cout << i << ": " << tileInfos[i].deltaRow << " " << tileInfos[i].deltaCol << " " << tileInfos[i].angle << "\n";
        }
    }
    // Test getPathsFromBitset()
    int paths[ALLDIM];
    board.getPathsFromBitset(paths);
    /** Testing for Ann **/
    /*
    int x[7][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0}};
    int y[7][3] = {{0,0,0},{1,1,1},{0,0,0},{1,1,1},{0,0,0},{1,1,1},{0,0,0}};

    int** xx = new int*[7];
    int** yy = new int*[7];

    for (int i = 0; i < 7; i++) {
        xx[i] = x[i];
        yy[i] = y[i];
    }

    Ann ann(3);
    ann.training(xx, yy, 7);
    ann.print();

    delete [] xx;
    delete [] yy;
    */
    return;
}
