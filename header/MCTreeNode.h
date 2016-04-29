/*
 * MCTreeNode.h
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */

#include <bitset>
#include <vector>
#include <utility>
#include "TraxRound.h"

using namespace std;

#ifndef MCTREENODE_H_
#define MCTREENODE_H_

struct MCTreeNode {
	unsigned long long int nodeId;
	bitset<1200> boardBitset;
	unsigned int winCount;
	unsigned int totalCount;
	vector<pair(unsigned long long int, unsigned long long int)> traxRounds;		// pair(gameId, roundId)
	vector<MCTreeNode*> parents;
	vector<unsigned long long int> parentIdGroup;
	vector<MCTreeNode*> children;
	vector<unsigned long long int> chidrenIdGroup;
	bool isWhite;
};

#endif /* MCTREENODE_H_ */
