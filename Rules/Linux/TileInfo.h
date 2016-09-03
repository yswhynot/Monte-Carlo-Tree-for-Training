#ifndef TILEINFO_H_
#define TILEINFO_H_

struct TileInfo {
	bool valid;
	bool attack; // Depends on valid
	bool endPoint; // Depends on valid
	int pos1; // Depends on endPoint
	int pos2; // Depends on endPoint
    int deltaRow; // Depends on endPoint
    int deltaCol; // Depends on endPoint
    int angle; // Depends on endPoint
	
};

#endif /* TILEINFO_H_ */
