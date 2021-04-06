#include "time_zone.h"
#include "../save/manifest.h"

#include <dialogs.h>
#include <stdio.h>
#include <kbd.h>
#include <string.h>
#include <stdlib.h>

DIALOG dialog_time_zone = {
	.Width = 155, .Height = 55, .CallBack = NoCallBack,
	.NumItems = 5,
	.Fields =  {
		{ /* Title */
			.Type = D_HEADER,
			.Flags = DF_SKIP,
			.f.dHeader = {.oText = 0, .lButton = BT_OK, .rButton = BT_CANCEL}
		},
		{ /* Text 1 */
			.Type = D_TEXT,
			.x0 = 5, .y0 = 15,
			.Flags = DF_SKIP,
			.f.dEdit = {.oText = 40}
		},
		{ /* Text 2 */
			.Type = D_TEXT,
			.x0 = 5, .y0 = 25,
			.Flags = DF_SKIP,
			.f.dEdit = {.oText = 63}
		},
		{ /* Hours - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 35,
			.Flags = 0,
			.f.dEdit = {.oText = 94, .bOffset = 0, .Flen = 3, .Dlen = 4}
		},
		{ /* Minutes - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 55, .y0 = 35,
			.Flags = 0,
			.f.dEdit = {.oText = 100, .bOffset = 8, .Flen = 3, .Dlen = 4}
		},
		{.f = {}} /* End Tag */
	}
};

const char TIME_ZONE_BUF [] = "Enter the UTC offset for your time zone\0If negative, enter the\0minus sign in the hours field.\0Hours\0Minutes";

/* dialog for setting the time zone */
static int run_time_zone_dialog (HANDLE manifest_handle, int wide_format) {
	char *invalid_input_reason = NULL;
	short tz;
	char request_buffer [16];
	char *hours_txt = &request_buffer [0];
	char *minutes_txt = &request_buffer [8];
	dialog_time_zone.TextOffset = (void *) TIME_ZONE_BUF - (void *) &dialog_time_zone;
	if (wide_format) {
		dialog_time_zone.Width = 225;
		dialog_time_zone.Height = 65;
		dialog_time_zone.Fields [4].x0 = 70;
	} else {
		dialog_time_zone.Width = 155;
		dialog_time_zone.Height = 55;
		dialog_time_zone.Fields [4].x0 = 55;
	}
	int res = get_time_zone (manifest_handle, &tz);
	if (res != MANIFEST_OK)
		return TZ_ERROR;
	short hours = tz / 60, minutes = tz % 60;
	if (minutes < 0) 
		minutes = - minutes;
	memset (request_buffer, 0, 16);
	sprintf (hours_txt, "%d", hours);
	sprintf (minutes_txt, "%d", minutes);
	try_again:
	res = Dialog (&dialog_time_zone, 2, 10, request_buffer, NULL);
	if (res == KEY_ENTER) {
		if (strlen (hours_txt) == 0 || strlen (minutes_txt) == 0) {
			invalid_input_reason = "Values must not be empty.";
			goto invalid_input;
		}
		minutes = atoi (minutes_txt);
		if (minutes < 0) {
			invalid_input_reason = "Minutes should be zero or a positive integer.";
			goto invalid_input;
		}
		hours = atoi (hours_txt);
		if (hours < 0)
			minutes = -minutes;
		tz = hours * 60 + minutes;
		res = set_time_zone (manifest_handle, tz);
		if (res != MANIFEST_OK)
			return TZ_ERROR;
		return TZ_OK;
	} else if (res == KEY_ESC)
		return TZ_CANCELLED;
	else if (res == -1)
		return TZ_MEM;
	invalid_input:
	DlgMessage ("Invalid Input", invalid_input_reason, BT_OK, BT_NONE);
	goto try_again;
}
