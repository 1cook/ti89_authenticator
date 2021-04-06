#ifndef TILE_H
#define TILE_H
#include <wingraph.h>

struct tile_state {
	char active [4][3];
	WINDOW windows [4][3];
};

#endif
