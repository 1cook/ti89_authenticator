#ifndef TILE_H
#define TILE_H
#include <wingraph.h>

struct tile_state {
	char active [4][3];
	WINDOW windows [4][3];
};

int draw_tile (struct tile_state *ts, unsigned short row, unsigned short col, const char *code, const char *label, int large);
void close_tile (struct tile_state *ts, unsigned short row, unsigned short col);
void destruct_tiles (struct tile_state *ts);

#endif
