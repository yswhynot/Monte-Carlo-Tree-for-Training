#ifndef DATABASE_H_
#define DATABASE_H_

#include "Board.h"

#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace System;
using namespace System::Data::SQLite;

typedef bitset<DIM> STATE;

#define ALLVARIATION false
#define MIX true

#define DBID 0
#define DBBOARD 1
#define DBRATE 2
#define DBNUM 3
#define DBWRATE 4
#define DBCLASS 5

#define NUMTHRESHOLD 1
#define TOTALCLASS 10

ref class Database {
public:
	Database();
	~Database();
	SQLiteConnection^ getDb();
	void createTrainingTxt(bool mix);
	bool createVariations();
	void saveFeatureMaps();
	bool updateDb(bool win, Board board, vector<STATE> states, bool allVariations);
private:
	void updateDbSingle(SQLiteCommand^ cmd, bool win, Board board, STATE state, bool mix);
	int checkVariation(vector<string> states, string state, int begin);
	bool updateDbVariation(Board board, STATE state, int rate, int num, bool mix);
	string bitsetToString(STATE state);
	void saveSeperateImages(Board board, STATE state, string filenameWhite, string filenameRed);
	void saveMixedImage(Board board, STATE state, string filenameColor);
	void saveFeatureMap(Board board, STATE state, string filenameMap);
	SQLiteConnection^ db;
};

#endif /* DATABASE_H_ */