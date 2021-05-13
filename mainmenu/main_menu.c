#include <statline.h>
#include "main_menu.h"
#include "../codes/hotp/hotp.h"
#include "../save/secret_file.h"
#include "../save/manifest.h"
#include "../save/util.h"


/* If any of these functions return zero, the menu_state struct 
 * can be considered in a broken state.
 * must call clear state immediately and exit the program
 */

/* returns the position of the selected column */
static unsigned short position (struct menu_state *ms) {
	return ms->top_pos + ms->col_len * ms->cur_row + ms->cur_col;
}

static void clear_single_tile (struct menu_state *ms, unsigned short row, unsigned short col) {
	if (ms->loaded_secrets [row][col] != H_NULL) {
		struct loaded_secret *ls = HeapDeref (ms->loaded_secrets [row][col]);
		if (ls != NULL && !ls->archived)
			HeapUnlock (ls->file_handle);
		HeapFree (ms->loaded_secrets [row][col]);
		ms->loaded_secrets [row][col] = H_NULL;
		ms->errors [row][col] = 0;
	}
}

/* frees the memory in the menu state. unlocks all locked files */
static void clear_state (struct menu_state *ms) {
	unsigned short row, col;
	for (row = 0; row < 4; row++)
	for (col = 0; col < 3; col++)
		clear_single_tile (ms, row, col);
	destruct_tiles (&ms->ts);
}

/* load a single file in manifest pos into tile x, y 
 * Returns 1 if successfull ( or at least unsuccessful for a reason that we can handle)
 * 0 otherwise
 */
static int load_single_file (struct menu_state *ms, unsigned short pos, unsigned short row, unsigned short col) {
	char file_name [9];
	int read_res = get_file (ms->manifest_file, file_name, pos);
	if (read_res == MANIFEST_OTHER_ERROR)
		return 0;
	if (read_res == MANIFEST_INVALID) {
		ms->errors [row][col] = 1;
		return 1;
	}
	ms->loaded_secrets [row][col] = HeapAlloc (sizeof (struct loaded_secret));
	if (ms->loaded_secrets [row][col] == H_NULL)
		return 0;
	if (HeapLock (ms->loaded_secrets [row][col]) == H_NULL)
		return 0;
	struct loaded_secret *ls = HeapDeref (ms->loaded_secrets [row][col]);
	if (ls == NULL)
		return 0;
	read_res = read_secret (ls, file_name);
	if (read_res == SECRET_MEM || read_res == SECRET_OTHER_ERROR)
		return 0;
	else if (read_res == SECRET_OK) {
		if (!ls->archived)
			if (HeapLock (ls->file_handle) == H_NULL)
				return 0;
		HeapUnlock (ms->loaded_secrets [row][col]);
		ms->errors [row][col] = 0;
		ls->next_update_at = -1;
		return 1;
	} else {
		HeapFree (ms->loaded_secrets [row][col]);
		ms->loaded_secrets [row][col] = H_NULL;
		ms->errors [row][col] = 2;
		return 1;
	} 
}

/* resets the menu state to the position putting the cursor at the top left and the position at top 
 * returns 0 on failure, 1 on success, 2 if no codes are in the manifest.
 */
static int reset_at_position (struct menu_state *ms, unsigned short top) {
	unsigned int row, col;
	clear_state (ms);
	unsigned short size = get_size (ms->manifest_file);
	if (size == 0) {
		clear_state (ms);
		return 2;
	}
	if (top >= size || (top % ms->col_len != 0)) {
		ms->top_pos = 0;
		top = 0;
	}
	ms->cur_row = 0;
	ms->cur_col = 0;
	unsigned short current_pos = top;
	for (row = 0; row < ms->row_len; row++)
	for (col = 0; col < ms->col_len; col++) {
		if (current_pos >= size)
			return 1;
		else {
			if (load_single_file (ms, current_pos, row, col) == 0)
				return 0;
			current_pos += 1;
		}
	}
	return 1;
}

static int update_single_code (struct menu_state *ms, unsigned short row, unsigned short col, int64_t current_time, int force_draw) {
	int code_change = 0;
	if (ms->errors [row][col]) {
		return draw_tile (&ms->ts, row, col, "Error", "Check file content", 0);
	}
	if (ms->loaded_secrets [row][col] != H_NULL) {
		if (HeapLock (ms->loaded_secrets [row][col]) != H_NULL) {
			struct loaded_secret *ls = HeapDeref (ms->loaded_secrets [row][col]);
			if (ls != NULL) {
				code_change = current_time >= ls->next_update_at;
				if (code_change) {
					int64_t period = current_time / ((int64_t)ls->period);
					hotp (ls->current_code, ls->secret, ls->secret_len, period,ls->code_len);
					ls->next_update_at = (period + 1) * ((int64_t)ls->period);
				}
				if (code_change || force_draw) {
					draw_tile (&ms->ts, row, col, ls->current_code, ls->label, ls->code_len <= 8);
				}
			}
		}
		HeapUnlock (ms->loaded_secrets [row][col]);
	}
	return code_change || force_draw;
}

/* updates all codes that need upated */
static int update_codes (struct menu_state *ms, int64_t current_time, int force_draw) {
	/* update all codes on the screen. forcing a draw change if needed
	 * also, redraw the last if needed in order to keep it appearing selected
	 */
	unsigned int row, col;
	unsigned int changed = 0;
	for (row = 0; row < ms->row_len; row++) {
		for (col = 0; col < ms->col_len; col++) {
			if (row != ms->cur_row || col != ms->cur_col) {
				unsigned int res = update_single_code (ms, row, col, current_time, force_draw);
				if (res == -1)
					return 0;
				changed |= res;
			}
		}
	}
	update_single_code (ms, ms->cur_row, ms->cur_col, current_time, changed || force_draw);
	return 1;
}

static int move_cursor_left (struct menu_state *ms) {
	int new_col = (ms->cur_col > 0) ? (ms->cur_col - 1) : ms->col_len - 1;
	if (ms->loaded_secrets [ms->cur_row][new_col] != H_NULL) {
		ms->cur_col = new_col;
		/* using -1 as the new time guarantees that the code will be drawn but not changed */
		return update_single_code (ms, ms->cur_row, ms->cur_col, -1, 1) != -1;
	}
	return 1;
}

static int move_cursor_right (struct menu_state *ms) {
	int new_col = (ms->cur_col < ms->col_len - 1) ? (ms->cur_col + 1) : 0;
	if (ms->loaded_secrets [ms->cur_row][new_col] != H_NULL) {
		ms->cur_col = new_col;
		/* using -1 as the new time guarantees that the code will be drawn but not changed */
		return update_single_code (ms, ms->cur_row, ms->cur_col, -1, 1) != -1;
	}
	return 1;
}

static int move_cursor_up (struct menu_state *ms) {
	if (ms->cur_row > 0) {
		/* case 1: cursor is at not in top position within menu; move up */
		ms->cur_row -= 1;
		return update_single_code (ms, ms->cur_row, ms->cur_col, -1, 1) != -1;
	} else if (ms->top_pos < ms->col_len) {
		/* case 2: cursor is at the top position and the top of the list; do nothing */
		return 1;
	} else {
		/* case 3: the cursor is at the top and the entire menu needs to be scrolled up by 1 position */
		/* step 1: remove bottom tiles */
		int row, col;
		for (col = 0; col < ms->col_len; col++)
			clear_single_tile (ms, ms->row_len - 1, col);
		/* step 2: move all tiles down by one position */
		for (row = ms->row_len - 2; row >= 0; row--)
			for (col = 0; col < ms->col_len; col++)
				ms->loaded_secrets [row + 1][col] = ms->loaded_secrets [row][col];
		/* step 3: increment top position upwards */
		ms->top_pos -= ms->col_len;
		/* step 4: fill top tiles */
		for (col = 0; col < ms->col_len; col++)
			if (!load_single_file (ms, ms->top_pos + col, 0, col))
				return 0;
		return update_codes (ms, -1, 1);
	}
}

static int move_cursor_down (struct menu_state *ms) {
	/* case 1: cursor is at not in bottom position within menu;
	 * move down only if an item is there 
	 */
	if (ms->cur_row < ms->row_len - 1) {
		if (ms->loaded_secrets [ms->cur_row + 1][ms->cur_col] != H_NULL) {
			ms->cur_row += 1;
			return update_single_code (ms, ms->cur_row, ms->cur_col, -1, 1) != -1;
		} else
			return 1;
	}
	unsigned short size = get_size (ms->manifest_file);
	/* case 2: already scrolled to the very bottom, no more to load. do nothing */
	unsigned short starting_new_tile = ms->top_pos + (ms->row_len * ms->col_len);
	if (starting_new_tile >= size)
		return 1;
	/* case 3: cursor is at the bottom and there is more to scroll onto 
	 * scroll down by one position
	 */
	/* step 1: remove top tiles */
	unsigned int row, col;
		for (col = 0; col < ms->col_len; col++)
			clear_single_tile (ms, 0, col);
	/* step 2: move all tiles up by one position */
	for (row = 0; row < ms->row_len - 1; row++)
		for (col = 0; col < ms->col_len; col++)
			ms->loaded_secrets [row][col] = ms->loaded_secrets [row + 1][col];
	/* step 3: increment top position upwards */
	ms->top_pos += ms->col_len;
	/* step 4: fill bottom tiles */
	for (col = 0; col < ms->col_len; col++) {
		if (starting_new_tile + col >= size) {
			ms->loaded_secrets [ms->row_len - 1][col] = H_NULL;
			close_tile (&ms->ts, ms->row_len - 1, col);
		} else {
			if (!load_single_file (ms, starting_new_tile + col, ms->row_len - 1, col))
				return 0;
		}
	}
	if (ms->loaded_secrets [ms->cur_row][ms->cur_col] == H_NULL)
		ms->cur_col = 0;
	/* step 5: draw all tiles in the new position */
	return update_codes (ms, -1, 1);
	
}
