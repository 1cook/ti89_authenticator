#include "new_secret.h"
#include "../save/util.h"
#include "../save/secret_file.h"
#include "../save/manifest.h"

#include <dialogs.h>
#include <alloc.h>
#include <kbd.h>
#include <string.h>
#include <stdlib.h>

/* 0xff : Invalid ; 0xfe : Ignore ; 0x20 Padding Character */
const unsigned char conversion_table [] = {
/*00*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
/*10*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
/*20*/	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
/*30*/	0xff, 0xff, 26  , 27  , 28  , 29  , 30  , 31  , 0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 0xff, 0xff,
/*40*/  0xff, 0   , 1   , 2   , 3   , 4   , 5   , 6   , 7   , 8   , 9   , 10  , 11  , 12  , 13  , 14  ,
/*50*/  15  , 16  , 17  , 18  , 19  , 20  , 21  , 22  , 23  , 24  , 25  , 0xff, 0xff, 0xff, 0xff, 0xff,
/*60*/  0xff, 0   , 1   , 2   , 3   , 4   , 5   , 6   , 7   , 8   , 9   , 10  , 11  , 12  , 13  , 14  ,
/*70*/  15  , 16  , 17  , 18  , 19  , 20  , 21  , 22  , 23  , 24  , 25  , 0xff, 0xff, 0xff, 0xff, 0xff
};

/* converts base32 string to binary
 * Returns length of converted binary string
 * or 0 if unsuccessful
 * Adapted from a similar function in CyoEncode Library
 */
unsigned short base_32 (unsigned char *dst, char *src) {
	unsigned short i, ii = 0;
	unsigned char *tmp_dst = dst;
	char tmp [8];
	for (i = 0 ; src [i] != '\0'; i += 1) {
		unsigned char ind = src [i];
		if (ind > 0x7f)
			return 0;
		unsigned char converted_character = conversion_table [ind];
		if (converted_character >= 32) {
			if (converted_character == 0xfe)
				continue; /* character is ignored */
			else if (converted_character == 32 && ii >= 2)
				converted_character = 0;
			else
				return 0;
			
		}
		tmp [ii++] = converted_character;
		if (ii == 8) {
			ii = 0;
			*tmp_dst++ = ((tmp [0] & 0x1f) << 3) | ((tmp [1] & 0x1c) >> 2);
			*tmp_dst++ = ((tmp [1] & 0x03) << 6) | ((tmp [2] & 0x1f) << 1) | ((tmp [3] & 0x10) >> 4);
			*tmp_dst++ = ((tmp [3] & 0x0f) << 4) | ((tmp [4] & 0x1e) >> 1);
			*tmp_dst++ = ((tmp [4] & 0x01) << 7) | ((tmp [5] & 0x1f) << 2) | ((tmp [6] & 0x18) >> 3);
			*tmp_dst++ = ((tmp [6] & 0x07) << 5) | (tmp [7] & 0x1f);
		}
	}
	if (ii == 0) {
		if (tmp_dst == dst) {
			return 0;
		} else {
			unsigned short base_size = tmp_dst - dst;
			int j, k;
			for (j = 7; j >= 0; j -= 1)
				if (tmp [j] != 0)
					break;
			for (k = j - 1; k >= 0; k -= 1)
				if (tmp [j] == 0)
					return 0;
			switch (j) {
				case 7:
					return base_size;
				case 6:
					return base_size - 1;
				case 4:
					return base_size - 2;
				case 3:
					return base_size - 3;
				case 1:
					return base_size - 4;
				default:
					return 0;
			}
		}
	} else {
		return 0;
	}
}

DIALOG dialog_new_file = {
	.Width = 120, .Height = 80, .CallBack = NoCallBack,
	.NumItems = 6,
	.Fields =  {
		{ /* Title */
			.Type = D_HEADER,
			.Flags = DF_SKIP,
			.f.dHeader = {.oText = 0, .lButton = BT_SAVE, .rButton = BT_CANCEL}
		},
		{ /* Secret - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 15,
			.Flags = DF_SCROLLABLE,
			.f.dEdit = {.oText = 11, .bOffset = 0, .Flen = 64, .Dlen = 12}
		},
		{ /* Label - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 25,
			.Flags = DF_SCROLLABLE,
			.f.dEdit = {.oText = 18, .bOffset = 68, .Flen = 16, .Dlen = 12}
		},
		{ /* Period - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 35,
			.Flags = 0,
			.f.dEdit = {.oText = 24, .bOffset = 88, .Flen = 3, .Dlen = 5}
		},
		{ /* Code Length - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 45,
			.Flags = 0,
			.f.dEdit = {.oText = 31, .bOffset = 96, .Flen = 2, .Dlen = 4}
		},
		{ /* File Name - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 55,
			.Flags = 0,
			.f.dEdit = {.oText = 43, .bOffset = 108, .Flen = 8, .Dlen = 10}
		},
		{.f = {}} /* End Tag */
	}
};
const char NEW_SECRET_TEXT_BUF [] = "New Secret\0Secret\0Label\0Period\0Code Length\0File Name";

char ERROR_TITLE [] = "Failed to save new secret.";
char ERROR_MEM [] = "You may not have enough memory.";
char ERROR_UNKNOWN [] = "An unknown error occured.";

int run_new_secret_dialog (HANDLE manifest_handle, int wide_format) {
	char request_buf [120];
	char *secret = &request_buf [0];
	char *label = &request_buf [68];
	char *period_txt = &request_buf [88];
	char *code_len_txt = &request_buf [96];
	char *file_name = &request_buf [108];
	short res;
	/* The way the OS chooses specify pointers to text in dialogs is absolutely bizzare */
	dialog_new_file.TextOffset = (void *) NEW_SECRET_TEXT_BUF - (void *) &dialog_new_file;
	if (wide_format) {
		dialog_new_file.Width = 150;
		dialog_new_file.Height = 90;
	} else {
		dialog_new_file.Width = 120;
		dialog_new_file.Height = 80;
	}
	memset (request_buf, 0, 120);
	/* set default values */
	strncpy (period_txt, "30", 3);
	strncpy (code_len_txt, "6", 2);
	try_prompt_again:
	res = Dialog (&dialog_new_file, 10, 10, request_buf, NULL);
	if (res == -1) {
		not_enough_mem:
		DlgMessage (ERROR_TITLE, ERROR_MEM, BT_OK, BT_NONE);
		return ND_MEM;
	}
	else if (res == KEY_ESC)
		return ND_CANCELLED;
	else if (res == KEY_ENTER) {
		char *invalid_input_msg = NULL;
		struct loaded_secret ls;
		char decoded_secret [64];
		/* set values of struct from dialog and validate entries */
		ls.secret = secret;
		ls.secret_len = strlen (secret); /* there's no strnlen in the romcalls */
		/* this should still be fine, because there there is padding between
		 * the end of each section and it is set to zero in advance
		 */
		ls.label = label;
		ls.label_len = strlen (label);
		if (ls.label_len == 0) {
			invalid_input_msg = "Label must not be empty.";
			goto invalid_input;
		}
		short tmp_short = atoi (period_txt);
		if (tmp_short < 1) {
			invalid_input_msg = "Period must be a positive integer.";
			goto invalid_input;
		}
		ls.period = tmp_short;
		tmp_short = atoi (code_len_txt);
		if (tmp_short < 6 || tmp_short > 10) {
			invalid_input_msg = "Code length must be between 6 and 10.";
			goto invalid_input;
		}
		ls.code_len = tmp_short;
		if (strlen (file_name) == 0) {
			invalid_input_msg = "File name must not be empty.";
			goto invalid_input;
		}
		ls.secret_len = base_32 (decoded_secret, secret);
		if (ls.secret_len == 0) {
			invalid_input_msg = "You may have mistyped the secret.";
			goto invalid_input;
		}
		ls.secret = decoded_secret;
		int file_result = write_secret (&ls, file_name);
		if (file_result == SECRET_ALREADY_EXISTS || file_result == SECRET_RESERVED_NAME) {
			invalid_input_msg = "Please choose a different file name.";
			goto invalid_input;
		} else if (file_result == SECRET_MEM)
			goto not_enough_mem;
		else if (file_result == SECRET_OK) {
			/* secret is saved, write to manifest */
			int manifest_result = add_file (manifest_handle, file_name);
			if (manifest_result == MANIFEST_OK) {
				/* This takes up more than one line. Since minimum OS is 207 for time functions anyways, this is fine */
				DlgMessage ("File Saved!", "Scroll to bottom to view code. Archive variable to prevent data loss. Generate back-up codes if your service offers it.", BT_OK, BT_NONE);
				return ND_OK;
			}
			else {
				DlgMessage (ERROR_TITLE, "File saved, but failed to write to manifest.", BT_OK, BT_NONE);
				if (manifest_result == MANIFEST_MEM)
					goto not_enough_mem;
				else
					return ND_ERROR;
			}
		}
		DlgMessage (ERROR_TITLE, ERROR_UNKNOWN, BT_OK, BT_NONE);
		return ND_ERROR;
		invalid_input:
		/* on invalid input, try again */
		DlgMessage ("Invalid Input", invalid_input_msg, BT_OK, BT_NONE);
		goto try_prompt_again;
	} else
		return ND_ERROR;
}

DIALOG dialog_open_file = {
	.Width = 120, .Height = 35, .CallBack = NoCallBack,
	.NumItems = 2,
	.Fields =  {
		{ /* Title */
			.Type = D_HEADER,
			.Flags = DF_SKIP,
			.f.dHeader = {.oText = 0, .lButton = BT_OK, .rButton = BT_CANCEL}
		},
		{ /* File Name - Text Box */
			.Type = D_EDIT_FIELD,
			.x0 = 5, .y0 = 15,
			.Flags = 0,
			.f.dEdit = {.oText = 12, .bOffset = 0, .Flen = 64, .Dlen = 12}
		},
		{.f = {}} /* End Tag */
	}
};

const char OPEN_SECRET_TEXT_BUF [] = "Open Secret\0File Name";

int run_open_secret_dialog (HANDLE manifest_handle, int wide_format) {
	char *invalid_input_msg = NULL;
	char request_buf [9];
	dialog_open_file.TextOffset = (void *) OPEN_SECRET_TEXT_BUF - (void *) &dialog_open_file;
	if (wide_format) {
		dialog_open_file.Width = 175;
		dialog_open_file.Height = 45;
	} else {
		dialog_open_file.Width = 120;
		dialog_open_file.Height = 35;
	}
	memset (request_buf , 0, 9);
	short res;
	try_prompt_again:
	res = Dialog (&dialog_open_file, 10, 10, request_buf, NULL);
	if (res == -1) {
		not_enough_mem:
		DlgMessage (ERROR_TITLE, ERROR_MEM, BT_OK, BT_NONE);
		return ND_MEM;
	}
	if (res == KEY_ESC)
		return ND_CANCELLED;
	else if (res == KEY_ENTER) {
		if (strlen (request_buf) == 0) {
			invalid_input_msg = "File name must not be empty";
			goto invalid_input;
		}
		struct loaded_secret ls;
		int secret_res = read_secret (&ls, request_buf);
		if (secret_res == SECRET_MEM)
			goto not_enough_mem;
		else if (secret_res == SECRET_FILE_NOT_FOUND) {
			invalid_input_msg = "File does not exist.";
			goto invalid_input;
		} else if (secret_res == SECRET_FILE_INVALID) {
			invalid_input_msg = "File has invalid contents.";
			goto invalid_input;
		} else if (secret_res == SECRET_OK) {
			secret_res = add_file (manifest_handle, request_buf);
			if (secret_res == MANIFEST_OK) {
				DlgMessage ("File Added!", "Scroll to bottom to view code.", BT_OK, BT_NONE);
				return ND_OK;
			} else {
				if (secret_res == MANIFEST_MEM) {
					goto not_enough_mem;
				} else if (secret_res == MANIFEST_DUPLICATE) {
					DlgMessage ("Failed to add new secret.", "This secret is already added.", BT_OK, BT_NONE);
					return ND_OK;
				} else {
					DlgMessage ("Failed to add new secret.", "File is valid, but could not add it to manifest.", BT_OK, BT_NONE);
					return ND_ERROR;
				}
			}
		} else {
			DlgMessage (ERROR_TITLE, ERROR_UNKNOWN, BT_OK, BT_NONE);
			return ND_ERROR;
		}
	}
	invalid_input:
		/* on invalid input, try again */
		DlgMessage ("Invalid Input", invalid_input_msg, BT_OK, BT_NONE);
		goto try_prompt_again;
}
