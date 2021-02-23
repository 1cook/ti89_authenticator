#include <wingraph.h>
#include <graph.h>

#include "tile.h"

/* listing of pixel locations in which to draw various elements
 * enumerated individualy for each cell 
 */
const WIN_RECT coords [4][3] = {
	{
		{10,10,80,30},
		{85,10,155,30},
		{160,10,230,30}
	},
	{
		{10,35,80,55},
		{85,35,155,55},
		{160,35,230,55}
	},
	{
		{10,60,80,80},
		{85,60,155,80},
		{160,60,230,80}
	},
	{
		{10,85,80,105},
		{85,85,155,105},
		{160,85,230,105}
	}
};

WINDOW *get_window (struct tile_state *ts, unsigned short row, unsigned short col) {
	if (!ts->active [row][col]) {
		if (WinOpen (&ts->windows [row][col], &coords [row][col], WF_SAVE_SCR)) {
			ts->active [row][col] = 1;
		}
		else
			return NULL;
	}
	return &ts->windows [row][col];
}

int draw_tile (struct tile_state *ts, unsigned short row, unsigned short col, const char *code, const char *label, int large) {
	WINDOW *w = get_window (ts, row, col);
	WinActivate (w);
	WinClr (w);
	if (w == NULL)
		return 0;
	if (large)
		WinFont (w, F_8x10);
	else
		WinFont (w, F_6x8);	
	WinStrXY (w, 2, 1, code);
	WinFont (w, F_4x6);
	WinStrXY (w, 2, 11, label);
	return 1;
}

void close_tile (struct tile_state *ts, unsigned short row, unsigned short col) {
	if (ts->active [row][col]) {
		WinClose (&ts->windows [row][col]);
		ts->active [row][col] = 0;
	}
}

void destruct_tiles (struct tile_state *ts) {
	unsigned short x,y;
	for (x = 0; x < 4; x++)
	for (y = 0; y < 3; y++)
		close_tile (ts, x, y);
}
