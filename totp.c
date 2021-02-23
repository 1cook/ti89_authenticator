#define NO_CALC_DETECT

#define USE_TI89

/* This version is required for the time functions */
#define MIN_AMS 207

#include <tigcclib.h>

#include "save/util.h"
#include "save/manifest.h"
#include "mainmenu/main_menu.h"
#include "dialog/time_zone.h"
#include "dialog/new_secret.h"
#include "time/time.h"

const char ERROR_MEM [] = "You may not have enough memory.";
const char NO_SECRETS [] = "No secrets.";

int guarantee_at_least_one_secret (HANDLE manifest_file, int wide_format) {
	/* special case: user has no secrets loaded. this happens always on first run 
	 * it can also happen on deletion
	 */
	unsigned short size = get_size (manifest_file);
	if (size == 0) {
		/* prompt the user to create or load a secret, or if they do not do it, then exit the program */
		if (KEY_ENTER == DlgMessage (NO_SECRETS, "You have no secrets loaded.\n Would you like to create a new one?", BT_YES, BT_NO)) {
			int res = run_new_secret_dialog (manifest_file, wide_format);
			if (res == ND_MEM || res == ND_ERROR)
				return -1;
			if (res == ND_CANCELLED)
				return 0;
		} else {
			if (KEY_ENTER == DlgMessage (NO_SECRETS, "Alternatively, would you like to load an existing secret?", BT_YES, BT_NO)) {
				int res = run_open_secret_dialog (manifest_file, wide_format);
				if (res == ND_MEM || res == ND_ERROR)
					return -1;
				if (res == ND_CANCELLED)
					return 0;
			} else {
				return 0;
			}
		}
	}
	return 1;
}

int main_loop (struct menu_state *ms, short time_zone, int wide_format) {
	/* we need this in order to determine the appropriate keyboard values 
	 * for non-common keys (keys defined differently on
	 * ti 89 and ti 92 / v200
	 */
	unsigned short key_up, key_left, key_right, key_down;
	if (wide_format) {
		/* This is a TI-92 or V200 */
		key_right = 340;
		key_left = 337;
		key_up = 338;
		key_down = 344;
	} else {
		/* This is a TI-89 */
		key_right = 344;
		key_left = 338;
		key_up = 337;
		key_down = 340;
	}
	
	
	unsigned short year, month, day, hour, minute, second;
	/* next update at */
	unsigned long next_update_at = 0;
	
	void *kq = kbd_queue ();
	
	for (;;) {
		unsigned short key;
		if (FiftyMsecTick >= next_update_at) {
			next_update_at = FiftyMsecTick += 20;
			/* get time */
			DateAndTime_Get (&year, &month, &day, &hour, &minute, &second);
			int64_t current_time = timestamp_from_civil (year, month, day, hour, minute, second);
			current_time -= time_zone * 60;
		
			/* update codes on screen 
			* on the first run, the last time updated variable f
			* or each loaded secret is -1, guaranteeing an update
			* even as force_update is set to 0
			*/
			if (update_codes (ms, current_time, 0) == 0)
				return EXIT_FAILURE;
			ST_busy (ST_IDLE);
		}
		/* Handle keypress */
		if (!OSdequeue (&key, kq)) {
			if (key == KEY_OFF || key == KEY_ON)
				off();/* power off the calculator */
			else if (key == KEY_ESC)
				return EXIT_SUCCESS;
			else if (key == key_left) {
				if (move_cursor_left (ms) == 0)
					return EXIT_FAILURE;
			} else if (key == key_right) {
				if (move_cursor_right (ms) == 0)
					return EXIT_FAILURE;	
			} else if (key == key_up) {
				if (move_cursor_up (ms) == 0)
					return EXIT_FAILURE;
			} else if (key == key_down) {
				if (move_cursor_down (ms) == 0)
						return EXIT_FAILURE;
			} else if (key == KEY_F1) {
				int res = run_new_secret_dialog (ms->manifest_file, wide_format);
				if (res == ND_MEM || res == ND_ERROR)
					return -1;
				if (reset_at_position (ms, ms->top_pos) == 0)
					return EXIT_FAILURE;
			} else if (key == KEY_F2) {
				int res = run_open_secret_dialog (ms->manifest_file, wide_format);
				if (res == ND_MEM || res == ND_ERROR)
					return -1;
				if (reset_at_position (ms, ms->top_pos) == 0)
					return EXIT_FAILURE;
			} else if (key == KEY_F3) {
				int res = DlgMessage ("Are you sure you want to remove this secret?", "If so, it will be removed only from display.\nStill must be removed manually on VAR-LINK.", BT_YES, BT_NO);
				if (res == KEY_ENTER) {
					remove_file (ms->manifest_file, position (ms));
					res = guarantee_at_least_one_secret (ms->manifest_file, wide_format);
					if (res == -1)
						return EXIT_FAILURE;
					if (res == 0)
						return EXIT_SUCCESS;
					if (reset_at_position (ms, ms->top_pos) == 0)
						return EXIT_FAILURE;
				}
			} else if (key == KEY_F4) {
				int res = run_time_zone_dialog (ms->manifest_file, wide_format);
				if (res == TZ_MEM || res == TZ_ERROR)
					return EXIT_FAILURE;
				get_time_zone (ms->manifest_file, &time_zone);
			}
		}
		idle ();
	}
}

void _main (void) {
	/* Initialization steps */
	const HARDWARE_PARM_BLOCK *hard = FL_getHardwareParmBlock ();
	
	/* Step 1: Make sure that directory exists */
	if (!create_directory()) {
		DlgMessage ("Failed to create secrets directory.", "You may not have enough memory.", BT_OK, BT_NONE);
		return;
	}
	
	/* Step 2: Initialize the Manifest File */
	struct menu_state ms;
	memset (&ms, 0, sizeof (struct menu_state));
	int error_code;
	ms.manifest_file = init_manifest (&error_code);
	if (ms.manifest_file == H_NULL) {
		char *manifest_failure_reason = NULL;
		switch (error_code) {
			case MANIFEST_INVALID:
				manifest_failure_reason = "The manifest file has invalid contents.";
				break;
			case MANIFEST_LOCKED:
				manifest_failure_reason = "The manifest file must neither be archived nor locked.";
				break;
			case MANIFEST_MEM:
				manifest_failure_reason = "You may not have enough free memory.";
				break;
			default:
				manifest_failure_reason = "Unknown error while loading manifest file.";
				break;
		}
		DlgMessage ("Failed to load manifest file.", manifest_failure_reason, BT_OK, BT_NONE);
		return;
	}
	
	/* Step 3: Check the system clock and time zone */
	{
		unsigned short year, month, day, hour, minute, second;
		DateAndTime_Get (&year, &month, &day, &hour, &minute, &second);
		if ((!IsClockOn ()) || year <= 2011) { /* 2011 is the year that RFC 6238 was published */
			if (KEY_ENTER == DlgMessage ("The system clock appears not set.", 
			"The system clock must be set to the correct time.\nWould you like to set it now?", BT_YES, BT_NO))
				DateAndTime_SetDlg ();
		}
	}
	
	int wide_format = hard->hardwareID == 1 || hard->hardwareID == 8;
	if (error_code == MANIFEST_NEWLY_CREATED)
		/* if the manifest file is newly created, then the user has not set their time zone */
		if (run_time_zone_dialog (ms.manifest_file, wide_format) != TZ_OK)
			exit (EXIT_FAILURE);

	short tz;
	if (get_time_zone (ms.manifest_file, &tz) != MANIFEST_OK)
		exit (EXIT_FAILURE);
		
	/* Step 4: Check for first secret */
	error_code = guarantee_at_least_one_secret (ms.manifest_file, wide_format);
	if (error_code == -1)
		exit (EXIT_FAILURE);
	if (error_code == 0)
		exit (EXIT_SUCCESS);
		
	/* Step 5: Set up main menu */
	if (wide_format) {
		ms.row_len = 4;
		ms.col_len = 3;
	} else {
		ms.row_len = 3;
		ms.col_len = 2;
	}
	{
		unsigned short pos;
		if (get_position (ms.manifest_file, &pos) != MANIFEST_OK)
			exit (EXIT_FAILURE);
		error_code = reset_at_position (&ms, pos);
		if (error_code == 0) {
			clear_state (&ms);
			exit (EXIT_FAILURE);
		}
	}
		
	/* Step 6: Enter Main Loop */
	error_code = main_loop (&ms, tz, wide_format);
	if (error_code == EXIT_SUCCESS)
		set_position (ms.manifest_file, ms.top_pos);
	
	/* Step 7: Clean up and exit */
	clear_state (&ms);
	exit (error_code);
}
