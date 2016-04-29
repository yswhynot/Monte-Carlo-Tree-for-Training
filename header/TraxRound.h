/*
 * TraxRound.h
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */

#include <bitset>

using namespace std;

#ifndef TRAXSTATE_H_
#define TRAXSTATE_H_

struct TraxRound {
	// One game play - unique id
	unsigned long long int gameId;

	// Current round number
	// 0 for the initial state
	// 1 for the 1st round
	unsigned int roundIndex;

	// Is White's turn (1) or the Red's turn (0) to play
	bool isWhite;

	// Current board representation
	// Count from top left to bottom right
	// Horizontal first
	// e.g.
	/* 100 010 000
	 * 000 110 000
	 * 000 000 000
	 */
	// boardBitset = 100010000000110000000000000
	bitset<1200> boardBitset;
};

#endif /* TRAXSTATE_H_ */
