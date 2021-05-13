#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <alloc.h>
#include <inttypes.h>
#include "../gfx/tile.h"

struct menu_state {
	/* handle for the manifest file */
	HANDLE manifest_file;
	/* handle pointing to struct of loaded secrets */
	HANDLE loaded_secrets [4][3];
	/* if there was an error in loading a particular tile */
	char errors [4][3];
	/* cursor position on screen */
	unsigned char cur_row, cur_col;
	unsigned char row_len, col_len;
	/* numeric position of top-left entry in manifest file */
	unsigned short top_pos;
	struct tile_state ts;
};

#endif
