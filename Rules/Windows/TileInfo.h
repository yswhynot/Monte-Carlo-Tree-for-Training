#ifndef TILEINFO_H_
#define TILEINFO_H_

struct TileInfo {
	bool valid;
	bool white;
	int pos1;
	int pos2;
    int deltaRow;
    int deltaCol;
    int angle;
	bool attack;
};

#endif /* TILEINFO_H_ */
