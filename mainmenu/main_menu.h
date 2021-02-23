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

/* If any of these functions return zero, the menu_state struct 
 * can be considered in a broken state.
 * must call clear state immediately and exit the program
 */


/* resets the menu state to the position putting the cursor at the top left and the position at top 
 * returns 0 on failure, 1 on success, 2 if no codes are in the manifest.
 */
int reset_at_position (struct menu_state *ms, unsigned short top);
unsigned short position (struct menu_state *ms);
/* frees the memory in the menu state. unlocks all locked files */
void clear_state (struct menu_state *ms);
/* updates all codes that need upated */
int update_codes (struct menu_state *ms, int64_t current_time, int force_draw);
int move_cursor_left (struct menu_state *ms);
int move_cursor_right (struct menu_state *ms);
int move_cursor_up (struct menu_state *ms);
int move_cursor_down (struct menu_state *ms);
#endif
