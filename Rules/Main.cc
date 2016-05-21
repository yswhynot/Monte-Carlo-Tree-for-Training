#include "Board.h"
#include "Ann.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <sqlite3.h>
#include <fstream>
using namespace std;

typedef bitset<DIM> STATE;

/** Function prototypes **/
void playGames(sqlite3* db, int nGame);
bool randomSteps(Board* board, vector<STATE>* states);
bool updateDb(sqlite3* db, bool win, vector<STATE> states);
string bitsetToString(STATE state);
void printStates(vector<STATE> states);
void printState(STATE state);
bool readDb(sqlite3* db, int num);
bool writeTrx(Board board, string filename);
void testCases();

int main() {
    srand(time(0));
    sqlite3* db;
    int ret = sqlite3_open("trax.db", &db);
    if (ret != 0) {
        cout << "Cannot open DB\n";
        return 0;
    }

    playGames(db, 100);

    //readDb(db, 30);

    sqlite3_close(db);

    //testCases();

    cout << "Press ENTER to continue...\n";
    getchar();
    return 0;
}

void playGames(sqlite3* db, int nGame) {
    for (int game = 1; game <= nGame; game++) {
        Board board;
        vector<STATE> states;

        bool win = randomSteps(&board, &states);

        if (updateDb(db, win, states)) {
            cout << "Successfully record game " << game << "\n";
            // Write trx
            stringstream ss;
            ss << "./game/" << game << ".trx";
            if (writeTrx(board, ss.str())) {
                cout << "Successfully create " << game << ".trx\n";
            }
        } else {
            cout << "Interrupte at game " << game << "\n";
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
            type[2] = state[bitStart];
            type[1] = state[bitStart + 1];
            type[0] = state[bitStart + 2];

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

bool updateDb(sqlite3* db, bool win, vector<STATE> states) {
    int ret = sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL);
    if (ret != 0) {
        cout << "Wrong executing SQL BEGIN\n";
        return false;
    }

    bool normal = true;
    for (int s = 0; s < states.size() && normal; s++) {
        // Check if record exist
        string sql = "SELECT * FROM winning_rate WHERE board = '" + bitsetToString(states[s]) + "';";
        char** result;
        int rRow, rCol;
        int ret = sqlite3_get_table(db, sql.c_str(), &result, &rRow, &rCol, NULL);
        if (ret != 0) {
            cout << "Error in searching\n";
            normal = false;
            break;
        }

        if (rRow == 0) {
            sqlite3_free_table(result);
            // Insert new data
            sql = "INSERT INTO winning_rate VALUES ('" + bitsetToString(states[s]) + "'," + (win? "1": "0") + ",1);";
            int ret = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
            if (ret != 0) {
                cout << "Error in insert new record\n";
                normal = false;
                break;
            }
        } else {
            // Get old rate
            int rate = atoi(result[rCol + 1]) + (win? 1: 0);
            int num = atoi(result[rCol + 2]) + 1;
            stringstream ssRate;
            ssRate << rate;
            stringstream ssNum;
            ssNum << num;
            sqlite3_free_table(result);
            sql = "UPDATE winning_rate SET rate =" + ssRate.str() + ", num =" + ssNum.str() + " WHERE board = '" + bitsetToString(states[s]) + "';";
            int ret = sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
            if (ret != 0) {
                cout << "Error in update record\n";
                normal = false;
                break;
            }
        }
    }
    if (normal) {
        int ret = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        if (ret != 0) {
            cout << "Fail to commit\n";
        } else {
            return true;
        }
    } else {
        int ret = sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        if (ret != 0) {
            cout << "Fail to rollback\n";
        }
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

bool readDb(sqlite3* db, int num) {
    string sql = "SELECT * FROM winning_rate ORDER BY num DESC, rate DESC;";
    char** result;
    int rRow, rCol;
    int ret = sqlite3_get_table(db, sql.c_str(), &result, &rRow, &rCol, NULL);
    if (ret != 0) {
        cout << "Error in getting data from database\n";
        return false;
    }

    if (rRow == 0) {
        sqlite3_free_table(result);
        cout << "No result found\n";
        return true;
    } else {
        num = (num > rRow)? rRow: num;
        for (int r = 1; r < num; r++) {
            cout << "Pattern " << r << ":\n";
            // Handle bitset
            STATE s;
            for (int bit = 0; bit < DIM; bit++) {
                if ((result[r * rCol][bit] - '0') == 0) {
                    s.reset(bit);
                } else {
                    s.set(bit);
                }
            }
            printState(s);
            // Handle data
            int rate = atoi(result[r * rCol + 1]);
            int rNum = atoi(result[r * rCol + 2]);
            double wRate = (double) rate / rNum;
            cout << "Winning number: " << rate << ", total number: " << rNum << "\n";
            printf("Winning rate: %.2f\n", wRate);
        }
    }
}

bool writeTrx(Board board, string filename) {
    ofstream ff(filename.c_str());
    if (!ff) {
        cout << "Cannot create " << filename << endl;
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
    cout << "Player "<< winner << " wins the game.\n";
    // Test check valid positions
    board.reset();
    winner = board.updateBoardByCommands(string("@0+ A0\\ A3/"));
    board.printType();
    int pos[TILENUM][4];
    int posCnt;
    int choiceCnt;
    board.getValidPos(pos, &posCnt, &choiceCnt);
    for (int i = 0; i < posCnt; i++) {
        cout << pos[i][0] << ": " << (pos[i][1] == 1? "+": "") << (pos[i][2] == 1? "/": "") << (pos[i][3] == 1? "\\": "") << "\n";
    }
    cout << "Number of possible choices: " << choiceCnt << "\n";
    cout << "\n";
    // Test output TileInfo
    TileInfo* tileInfos;
    tileInfos = board.getTileInfos(true);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
            cout << i << ": " << tileInfos[i].deltaRow << " " << tileInfos[i].deltaCol << " " << tileInfos[i].angle << "\n";
        }
    }
    cout << "\n";
    tileInfos = board.getTileInfos(false);
    for (int i = 0; i < TILENUM; i++) {
        if (tileInfos[i].valid) {
            cout << i << ": " << tileInfos[i].deltaRow << " " << tileInfos[i].deltaCol << " " << tileInfos[i].angle << "\n";
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
