/*
 * MCTree.h
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */
#include <bitset>
#include <vector>
#include "TraxRound.h"
#include "MCTreeNode.h"

using namespace std;

#ifndef MCTREE_H_
#define MCTREE_H_

class MCTree {
public:
	// Operational Functions: return success=1, or fail=0
	bool insertRound(TraxRound &inputRound);
	bool insertGame(vector<TraxRound> &inputGame);

	// Info Functions
	unsigned long int getWinCount(bitset<1200> &inputBoardBit);
	unsigned long int getTotalCount(bitset<1200> &inputBoardBit);



private:
	vector<MCTreeNode> myTree;
};

#endif /* MCTREE_H_ */
