/*
 * MCTreeNode.h
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */

#include <bitset>
#include <unordered_set>
#include <vector>
#include "TraxRound.h"

using namespace std;

#ifndef MCTREENODE_H_
#define MCTREENODE_H_

struct MCTreeNode {
	bitset<1200> boardBitset;
	unsigned int winCount;
	unsigned int totalCount;
	unordered_set<TraxRound> traxRounds;
	MCTreeNode *parent;
	vector<*MCTreeNode> children;
//	MCTreeNode *leftSibling;
//	MCTreeNode *rightSibling;
};

#endif /* MCTREENODE_H_ */
