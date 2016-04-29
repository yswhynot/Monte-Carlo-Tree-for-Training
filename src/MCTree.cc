/*
 * MCTree.cc
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */

#include <stdio.h>
#include "MCTree.h"

using namespace std;

MCTree::MCTree() {
	MCTreeNode root_white = {
			0,	// node Id
			bitset<1200>, // boardBitset
			0, // winCount
			0, // totalCount
			nullptr, // traxRounds
			nullptr, // parent
			nullptr,	// parent Id
			nullptr,	// children
			nullptr,	// children id group
			true	// isWhite
	};
	MCTreeNode root_black = {
			1,	// node Id
			bitset<1200>, // boardBitset
			0, // winCount
			0, // totalCount
			nullptr, // traxRounds
			nullptr, // parent
			nullptr,	// parent Id
			nullptr,	// children
			nullptr,	// children id group
			false	// isWhite
	};

	myTree.push_back(root_white);
	myTree.push_back(root_black);

	totalNode = 2;
}

bool MCTree::updateParents(MCTreeNode &childNode, int winIncre, int totalIncre) {
	vector<MCTreeNode> it;

	if (childNode.parents == nullptr)
		return false;

	for (it = childNode.parents.begin(); it != childNode.parents.end(); it++) {
		*it.totalCount += totalIncre;
		*it.winCount += winIncre;

		// update parents recursively till root
		this->updateParents(*it, winIncre, totalIncre);
	}
	return true;
}

bool MCTree::updateNode(TraxRound &inputRound, unsigned long long int nodeId, int winIncre, int totalIncre) {

	MCTreeNode tmpNode = myTree[nodeId];
	tmpNode.totalCount += totalIncre;
	tmpNode.winCount += winIncre;
	tmpNode.traxRounds.push_back(make_pair(inputRound.gameId, inputRound.roundIndex));

	this->updateParents(tmpNode, winIncre);

	printf("Updated node of id %u", nodeId);
	return true;
}

MCTreeNode MCTree::insertNode(TraxRound &inputRound, vector<MCTreeNode*> &parentNodes, int winIncre, int totalIncre) {
	// create node
	MCTreeNode insertNode;
	insertNode.nodeId = totalNode++;
	insertNode.boardBitset = inputRound.boardBitset;
	insertNode.winCount += winIncre;
	insertNode.totalCount = totalIncre;
	insertNode.traxRounds.push_back(make_pair(inputRound.gameId, inputRound.roundIndex));
	insertNode.parents = parentNodes;
	insertNode.isWhite = inputRound.isWhite;

	// insert parent ids
	vector<MCTreeNode*> it;
	for (it = parentNodes.begin(); it != parentNodes.end(); it++)
		insertNode.parentIdGroup.push_back(it.nodeId);

	// update corresponding parents
	for (it = parentNodes.begin(); it != parentNodes.end(); it++) {
		it.children.push_back(insertNode);
		it.chidrenIdGroup.push_back(insertNode.nodeId);
	}

	return insertNode;
}

bool MCTree::insertGame(vector<TraxRound> &inputGame, bool isWhiteWin) {
	vector<TraxRound>::iterator itInput;

	for(itInput = inputGame.begin(); itInput != inputGame.end(); itInput++) {

		vector<MCTreeNode>::iterator itTree;
		MCTreeNode prevExistNode;

		for(itTree = myTree.begin(); itTree != myTree.end(); itTree++) {
			int winIncre = (*itInput.isWhite ^ isWhiteWin) ? (int)0 : (int)1;

			// if find board bitset exists, update node
			if (*itInput.isWhite == *itTree.isWhite && *itInput.boardBitset == *itTree.boardBitset)
				prevExistNode = this->updateNode(*itInput, *itTree.nodeId, winIncre, 1);
			else {
				// board bitset not exists, insert node
				// its parent should exist already
				vector<MCTreeNode*> parentNode(1, &prevExistNode);
				this->insertNode(*itInput, parentNode, winIncre, 1);
			}	// end else
		}	// end for
	}	// end for
	return true;
}

bool MCTree::mergeTree(MCTree &inputTree) {
	vector<MCTreeNode> itInput;

	for (itInput = inputTree.begin(); itInput != inputTree.end(); itInput++) {

		vector<MCTreeNode>::iterator itTree;
		for(itTree = myTree.begin(); itTree != myTree.end(); itTree++) {

			// if find board bitset exists, update node
			if (*itInput.isWhite == *itTree.isWhite && *itInput.boardBitset == *itTree.boardBitset)
				this->updateNode(*itInput, *itTree.nodeId, *itInput.winCount, *itInput.totalCount);
			else {
				// board bitset not exists, insert node
				// its parent should exist already
				this->insertNode(*itInput, *itInput.parents, *itInput.winCount, *itInput.totalCount);
			}	// end else
		}	// end for
	}	// end for

	return true;
}

bool MCTree::loadTree(string &path) {
	return true;
}
