#include "Database.h"

Database::Database() {
	this->db = gcnew SQLiteConnection();
	this->db->ConnectionString = "Data Source = trax.db";
	this->db->Open();
}

Database::~Database() {
	db->Close();
	delete (IDisposable^)db;
}

SQLiteConnection^ Database::getDb() {
	return this->db;
}

bool Database::updateDb(bool win, Board board, vector<STATE> states, bool allVariations) {
	SQLiteCommand^ cmd = this->db->CreateCommand();
	SQLiteTransaction^ tx = this->db->BeginTransaction();
	cmd->Transaction = tx;

	try {
		for (int s = 0; s < states.size(); s++) {
			if (allVariations == false) {
				this->updateDbSingle(cmd, win, board, states[s], MIX);
			}
			else {
				// Original
				this->updateDbSingle(cmd, win, board, states[s], MIX);
				// Original with 90 degree clockwise turn
				board.loadBoardFromString(this->bitsetToString(states[s]));
				board.clockwise();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
				// Original with 180 degree clockwise turn
				board.clockwise();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
				// Original with 270 degree clockwise turn
				board.clockwise();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
				// Flip
				board.clockwise();
				board.flip();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
				// Flip with 90 degree clockwise turn
				board.clockwise();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
				// Flip with 180 degree clockwise turn
				board.clockwise();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
				// Flip with 270 degree clockwise turn
				board.clockwise();
				this->updateDbSingle(cmd, win, board, board.getBoardBitset(), MIX);
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

void Database::updateDbSingle(SQLiteCommand^ cmd, bool win, Board board, STATE state, bool mix) {
	// Check if record exists
	string sql = "SELECT * FROM winning_rate WHERE board = '" + this->bitsetToString(state) + "';";
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
		sql = "UPDATE winning_rate SET rate =" + ssRate.str() + ", num =" + ssNum.str() + " WHERE board = '" + this->bitsetToString(state) + "';";
		cmd->CommandText = gcnew String(sql.c_str());
		cmd->ExecuteNonQuery();
	}
	else {
		// Finish reading
		reader->Close();
		// Insert new data
		sql = "INSERT INTO winning_rate VALUES (NULL, '" + this->bitsetToString(state) + "'," + (win ? "1" : "0") + ",1);";
		cmd->CommandText = gcnew String(sql.c_str());
		cmd->ExecuteNonQuery();
		// Save a new image
		sql = "SELECT id FROM winning_rate WHERE board = '" + this->bitsetToString(state) + "';";
		cmd->CommandText = gcnew String(sql.c_str());
		reader = cmd->ExecuteReader();
		reader->Read();
		int id = reader->GetInt32(0);
		reader->Close();
		if (mix) {
			stringstream ssColor;
			ssColor << "./image/M" << id << ".bmp";
			string filenameColor = ssColor.str();
			stringstream ssMap;
			ssMap << "./map/M" << id << ".bmp";
			string filenameMap = ssMap.str();
			this->saveMixedImage(board, state, filenameColor, filenameMap);
		}
		else {
			stringstream ssWhite;
			ssWhite << "./image/W" << id << ".bmp";
			string filenameWhite = ssWhite.str();
			stringstream ssRed;
			ssRed << "./image/R" << id << ".bmp";
			string filenameRed = ssRed.str();
			this->saveSeperateImages(board, state, filenameWhite, filenameRed);
		}
	}
}

string Database::bitsetToString(STATE state) {
	string ret = "";
	for (int i = 0; i < DIM; i++) {
		ret += state.test(i) ? "1" : "0";
	}
	return ret;
}

void Database::saveSeperateImages(Board board, STATE state, string filenameWhite, string filenameRed) {
	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
	board.loadBoardFromString(bitsetToString(state));
	board.imageOutput(whiteImage, redImage);
	cv::imwrite(filenameWhite, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, whiteImage));
	cv::imwrite(filenameRed, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, redImage));
}

void Database::saveMixedImage(Board board, STATE state, string filenameColor, string filenameMap) {
	board.loadBoardFromString(bitsetToString(state));

	unsigned char whiteImage[OUTPUTWIDTH * OUTPUTWIDTH];
	unsigned char redImage[OUTPUTWIDTH * OUTPUTWIDTH];
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
	cv::imwrite(filenameColor, img);

	unsigned char map[OUTPUTWIDTH * OUTPUTWIDTH];
	board.mapOutput(map);
	cv::imwrite(filenameMap, cv::Mat(OUTPUTWIDTH, OUTPUTWIDTH, CV_8UC1, map));
}

void Database::createTrainingTxt(bool mix) {
	/** Pre-handle on database **/
	SQLiteCommand^ cmd = this->db->CreateCommand();
	SQLiteTransaction^ tx = this->db->BeginTransaction();
	cmd->Transaction = tx;
	// Add column wRat
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
	/** Create TXTs**/
	sql = "SELECT * FROM winning_rate;";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();

	if (reader->HasRows) {
		if (mix) {
			ofstream fTest("./txt/test.txt");
			ofstream fTrain("./txt/train.txt");
			ofstream fVal("./txt/val.txt");
			int cnt = 0;
			while (reader->Read()) {
				stringstream ss;
				ss << "M" << reader->GetInt32(DBID) << ".jpeg " << reader->GetInt32(DBCLASS) << endl;
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
			std::cout << "Sucessfully create TXTs\n";
		}
		else {
			ofstream fTestWhite("./txt/Wtest.txt");
			ofstream fTrainWhite("./txt/Wtrain.txt");
			ofstream fValWhite("./txt/Wval.txt");
			ofstream fTestRed("./txt/Rtest.txt");
			ofstream fTrainRed("./txt/Rtrain.txt");
			ofstream fValRed("./txt/Rval.txt");
			int cnt = 0;
			while (reader->Read()) {
				stringstream ssWhite;
				ssWhite << "W" << reader->GetInt32(DBID) << ".jpeg " << reader->GetInt32(DBCLASS) << endl;
				stringstream ssRed;
				ssRed << "R" << reader->GetInt32(DBID) << ".jpeg " << TOTALCLASS - 1 - reader->GetInt32(DBCLASS) << endl;
				if (cnt % 4 == 0) {
					fTestWhite << ssWhite.str();
					fTestRed << ssRed.str();
				}
				else if (cnt % 5 == 0) {
					fValWhite << ssWhite.str();
					fValRed << ssRed.str();
				}
				else {
					fTrainWhite << ssWhite.str();
					fTrainRed << ssRed.str();
				}
				// Update counter
				cnt++;
			}
			// Finish reading
			reader->Close();
			// Close files
			fTestWhite.close();
			fTrainWhite.close();
			fValWhite.close();
			fTestRed.close();
			fTrainRed.close();
			fValRed.close();
			std::cout << "Sucessfully create TXTs\n";
		}
	}
	else {
		// Finish reading
		reader->Close();
		std::cout << "No result found\n";
	}
	tx->Rollback();
}

bool Database::createVariations() {
	// Read all states in the database
	vector<string> states;
	vector<int> rates;
	vector<int> nums;

	SQLiteCommand^ cmd = this->db->CreateCommand();
	string sql = "SELECT * FROM winning_rate;";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();
	if (reader->HasRows) {
		while (reader->Read()) {
			// Read single row of data
			char* sStr = (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(reader->GetString(DBBOARD));
			string state(sStr);
			int rate = reader->GetInt32(DBRATE);
			int num = reader->GetInt32(DBNUM);
			// Store in vector
			states.push_back(state);
			rates.push_back(rate);
			nums.push_back(num);
		}
		// Finish reading
		reader->Close();
	}
	else {
		// Finish reading
		reader->Close();
		std::cout << "No result found\n";
		return false;
	}

	cmd = this->db->CreateCommand();
	SQLiteTransaction^ tx = this->db->BeginTransaction();
	cmd->Transaction = tx;
	try {
		for (int s = 0; s < states.size(); s++) {
			Board board;
			// Original with 90 degree clockwise turn
			board.loadBoardFromString(states[s]);
			board.clockwise();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
			// Original with 180 degree clockwise turn
			board.clockwise();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
			// Original with 270 degree clockwise turn
			board.clockwise();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
			// Flip
			board.clockwise();
			board.flip();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
			// Flip with 90 degree clockwise turn
			board.clockwise();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
			// Flip with 180 degree clockwise turn
			board.clockwise();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
			// Flip with 270 degree clockwise turn
			board.clockwise();
			this->copyDbSingle(cmd, board, board.getBoardBitset(), rates[s], nums[s], MIX);
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

void Database::copyDbSingle(SQLiteCommand^ cmd, Board board, STATE state, int rate, int num, bool mix) {
	// Check if record exists
	string sql = "SELECT * FROM winning_rate WHERE board = '" + this->bitsetToString(state) + "';";
	cmd->CommandText = gcnew String(sql.c_str());
	SQLiteDataReader^ reader = cmd->ExecuteReader();
	if (reader->HasRows == false) {
		// Finish reading
		reader->Close();
		// Insert new data
		stringstream ss;
		ss << "INSERT INTO winning_rate VALUES (NULL, '" << this->bitsetToString(state) << "'," << rate << "," << num << ");";
		sql = ss.str();
		cmd->CommandText = gcnew String(sql.c_str());
		cmd->ExecuteNonQuery();
		// Save a new image
		sql = "SELECT id FROM winning_rate WHERE board = '" + this->bitsetToString(state) + "';";
		cmd->CommandText = gcnew String(sql.c_str());
		reader = cmd->ExecuteReader();
		reader->Read();
		int id = reader->GetInt32(0);
		// Finish reading
		reader->Close();
		if (mix) {
			stringstream ssColor;
			ssColor << "./image/M" << id << ".bmp";
			string filenameColor = ssColor.str();
			stringstream ssMap;
			ssMap << "./map/M" << id << ".bmp";
			string filenameMap = ssMap.str();
			this->saveMixedImage(board, state, filenameColor, filenameMap);
		}
		else {
			stringstream ssWhite;
			ssWhite << "./image/W" << id << ".bmp";
			string filenameWhite = ssWhite.str();
			stringstream ssRed;
			ssRed << "./image/R" << id << ".bmp";
			string filenameRed = ssRed.str();
			this->saveSeperateImages(board, state, filenameWhite, filenameRed);
		}
	}
	else {
		// Finish reading
		reader->Close();
	}
}