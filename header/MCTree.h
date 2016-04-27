/*
 * MCTree.h
 *
 *  Created on: Apr 27, 2016
 *      Author: yisha
 */

#ifndef MCTREE_H_
#define MCTREE_H_

class MCTree {
public:
	bool insertNode();
	bool insertBranch();
	int getTreeHeight();

private:
	int height;
};

#endif /* MCTREE_H_ */
