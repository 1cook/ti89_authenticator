#include "manifest.h"

#include <vat.h>
#include <estack.h>
#include <string.h>

char MANIFEST_FILE [] = "\0secrets\\manifest";
char *MANIFEST_FILE_P = MANIFEST_FILE + sizeof (MANIFEST_FILE) - 1;

char MANIFEST_TYPE_TAG [] = {0, 'M', 'A', 'N', 'F', 0, OTH_TAG};
char MANIFEST_HEAD [] = {'M', 'A', 'N', 'F'};

/* call create_directory of util.h before calling any of these
 * below functions
 */

/* loads the manifest file, creates if it does not already exists
 * returns the handle to the file or H_NULL if unsuccessful
 * 
 * this does not guarantee that the files listed in the manifest
 * actually exist or are valid
 */
static HANDLE init_manifest (int *error_code) {
	HSym hs = SymFind (MANIFEST_FILE_P);
	if (hs.folder == 0) {
		/* the file does not exists. must now create it */
		/* allocate space for the manifest */
		HANDLE handle = HeapAlloc (18);
		if (handle == H_NULL) {
			*error_code = MANIFEST_MEM;
			return H_NULL;
		}
		MULTI_EXPR *calc_var = HeapDeref (handle);
		if (calc_var == NULL) {
			*error_code = MANIFEST_OTHER_ERROR;
			return H_NULL;
		}
		/* set the file to the initial state with zero members */
		calc_var->Size = 16;
		memcpy (calc_var->Expr, MANIFEST_HEAD, 4);
		memset (calc_var->Expr + 4, 0, 5);
		memcpy (calc_var->Expr + 9, MANIFEST_TYPE_TAG, 7);
		/* actually create the file entry */
		hs = SymAdd (MANIFEST_FILE_P);
		if (hs.folder == 0) {
			*error_code = MANIFEST_OTHER_ERROR;
			goto creation_error;
		}
		SYM_ENTRY *se = DerefSym (hs);
		if (se == NULL) {
			*error_code = MANIFEST_OTHER_ERROR;
			HSymDel (hs);
			goto creation_error;
		}
		*error_code = MANIFEST_NEWLY_CREATED;
		se->handle = handle;
		return handle;
		creation_error:
		HeapFree (handle);
		return H_NULL;
	} else {
		/* the file exists. must now check for validity */
		SYM_ENTRY *se = DerefSym (hs);
		if (se == NULL) {
			*error_code = MANIFEST_OTHER_ERROR;
			return H_NULL;
		}
		/* we need to be able to modify the file in order to swap
		 * the positions of various entry and change the ordering of
		 * codes on screen. We also need this in order to set the
		 * current position in the list therefore the file must neither be locked
		 * nor archived (in flash). This does mean that the file will be 
		 * lost on reset, but the file can easily be rebuilt after then
		 * without having to reenter secrets because secret files
		 * are allowed to be archived.
		 */
		if (se->flags.bits.archived || se->flags.bits.locked) {
			*error_code = MANIFEST_LOCKED;
			return H_NULL;
		} else if (se->flags.bits.folder) {
			*error_code = MANIFEST_INVALID;
			return H_NULL;
		}
		MULTI_EXPR *calc_var = HeapDeref (se->handle);
		if (calc_var == NULL) {
			*error_code = MANIFEST_OTHER_ERROR;
			return H_NULL;
		}
		/* The file size at the beginning is used to determine the size
		* of the file instead of the size of the allocated space, but
		* the size of the allocated space still has to be greater or equal
		* to the stated size (plus 2 bytes for the size itself)
		*/	
		if ((calc_var->Size + 2) > HeapSize (se->handle)) {
			*error_code = MANIFEST_INVALID;
			return H_NULL;
		}
		/* blocks in this file are 8 bytes in length
		 * the first block is the header, which contains the characters
		 * "MANF", the timezone in minutes (2 bytes), followed by the
		 * index of the currently selected item (2 bytes)
		 * subsequent blocks contain file names for secrets file.
		 * the last file is the 0 byte, followed by the 7 byte file type tag
		 */
		if (calc_var->Size % 8 != 0) {
			*error_code = MANIFEST_INVALID;
			return H_NULL;
		}
		/* There must be 2 blocks at least.
		 * the first and the last.
		 * the first is the header, the last the tag
		 */
		if ((calc_var->Size / 8) < 2) {
			*error_code = MANIFEST_INVALID;
			return H_NULL;
		}
		/* check for the presence of the header */
		if (memcmp (calc_var->Expr, MANIFEST_HEAD, sizeof (MANIFEST_HEAD)) != 0) {
			*error_code = MANIFEST_INVALID;
			return H_NULL;
		}
		/* check for a valid position. it must not be greater than
		 * or equal to the total number of entries.
		 * alternatively, there could simply not be any members and the 
		 * position does not matter.
		 */
		unsigned short total_entries = (calc_var->Size / 8) - 2;
		unsigned short position;
		memcpy (&position, calc_var->Expr + 6, 2);
		if (total_entries > 0 && position >= total_entries) {
			*error_code = MANIFEST_INVALID;
			return H_NULL;
		}
		*error_code = MANIFEST_OK;
		return se->handle;
	}
}

/* tz is the timezone utc offset expressed in minutes 
 * returns one of the error codes
 */
static int set_time_zone (HANDLE manifest_handle, short tz) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	memcpy (calc_var->Expr + 4, &tz, 2);
	return MANIFEST_OK;
}

/* like set_time_zone. tz will be filled with the current time zone
 */
static int get_time_zone (HANDLE manifest_handle, short *tz) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	memcpy (tz, calc_var->Expr + 4, 2);
	return MANIFEST_OK;
}

/* these are just like set_time_zone and get_time_zone respectively */
static int set_position (HANDLE manifest_handle, unsigned short pos) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	memcpy (calc_var->Expr + 6, &pos, 2);
	return MANIFEST_OK;
}

static int get_position (HANDLE manifest_handle, unsigned short *pos) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	memcpy (pos, calc_var->Expr + 6, 2);
	return MANIFEST_OK;
}

/* adds a filename onto the end of the manifest file */
static int add_file (HANDLE manifest_handle, char *file_name) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	/* we have this limit because the maximum file size is 65535
	 * bytes. Most people will not have this many secrets 
	 */
	unsigned short size = calc_var->Size / 8 - 2;
	if (size >= 8000)
		return MANIFEST_TOO_MANY_FILES;
	/* check for duplicates */
	unsigned short i;
	for (i = 0; i < size; i++) {
		if (strncmp (file_name, calc_var->Expr + 8 + (i * 8), 8) == 0)
			return MANIFEST_DUPLICATE;
	}
	/* 8 bytes for the new file, 2 file size */
	HANDLE res = HeapRealloc (manifest_handle, calc_var->Size + 10);
	if (res == H_NULL)
		return MANIFEST_MEM;
	/* now that we have reallocated, we must dereference one again */
	calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	/* an 8 byte filename will not be copied with a zero terminator.
	 * this is okay. we have code that will properly add the terminator
	 * upon retrieval
	 */
	strncpy (calc_var->Expr + calc_var->Size - 8, file_name, 8);
	memcpy (calc_var->Expr + calc_var->Size + 1, MANIFEST_TYPE_TAG, 7);
	calc_var->Size += 8;
	return MANIFEST_OK;	
}

/* gets a file name from the manifest file 
 * file name points to a buffer that is at least nine bytes long.
 * will write the name of the file to it, adding zero terminator.
 */
static int get_file (HANDLE manifest_handle, char *file_name, unsigned short index) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	if (index >= ((calc_var->Size / 8) - 2))
		return MANIFEST_OTHER_ERROR;
	memcpy (file_name, calc_var->Expr + 8 + (index * 8), 8);
	file_name [8] = '\0';
	if (strlen (file_name) == 0)
		return MANIFEST_INVALID;
	return MANIFEST_OK;
}

static int remove_file (HANDLE manifest_handle, unsigned short index) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	if (index >= ((calc_var->Size / 8) - 2))
		return MANIFEST_OTHER_ERROR;
	/* move the entire part of the file after the removed entry
	 * 8 bytes to fill the space of the removed file.
	 * This moves everything after the removed file, including the tag.
	 */
	memcpy (calc_var->Expr + 8 + (index * 8), calc_var->Expr + 16 + (index * 8),
		calc_var->Size - (8 + (index * 8)));
	calc_var->Size -= 8;
	/* now resize the allocated block to free up memory */
	if (H_NULL == HeapRealloc (manifest_handle, calc_var->Size + 2))
		/* this will likely never happen, due to reduction in size */
		return MANIFEST_MEM;
	return MANIFEST_OK;
}

/* swaps the positions of one file with another in the manifest */
static int swap_positions (HANDLE manifest_handle, unsigned short pos1, unsigned short pos2) {
	char tmp [8];
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return MANIFEST_OTHER_ERROR;
	unsigned short size = calc_var->Size / 8 - 2;
	if (pos1 >= size || pos2 >= size)
		return MANIFEST_OTHER_ERROR;
	memcpy (tmp, calc_var->Expr + 8 + (pos1 * 8), 8);
	memcpy (calc_var->Expr + 8 + (pos1 * 8), calc_var->Expr + 8 + (pos2 * 8), 8);
	memcpy (calc_var->Expr + 8 + (pos2 * 8), tmp, 8);
	return MANIFEST_OK;
}

static unsigned short get_size (HANDLE manifest_handle) {
	MULTI_EXPR *calc_var = HeapDeref (manifest_handle);
	if (calc_var == NULL)
		return 0;
	return calc_var->Size / 8 - 2;
}
