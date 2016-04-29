/*
 * MCTree.h
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */
#include <bitset>
#include <vector>
#include <string>
#include "TraxRound.h"
#include "MCTreeNode.h"

using namespace std;

#ifndef MCTREE_H_
#define MCTREE_H_

class MCTree {
public:
	MCTree();
	~MCTree();

	// Operational Functions: return success=1, or fail=0
	bool insertGame(vector<TraxRound> &inputGame, bool isWhiteWin);
	bool mergeTree(MCTree &inputTree);
	bool loadTree(string &path);

	// Info Functions
	unsigned long int getWinCount(bitset<1200> &inputBoardBit);
	unsigned long int getTotalCount(bitset<1200> &inputBoardBit);



private:
	vector<MCTreeNode> myTree;
	unsigned long long int totalNode;

	bool updateNode(TraxRound &inputRound, unsigned long long int nodeId, bool isWhiteWin);
	MCTreeNode insertNode(TraxRound &inputRound, bool isWhiteWin);
	bool updateParents(MCTreeNode &childNode);
};

#endif /* MCTREE_H_ */
