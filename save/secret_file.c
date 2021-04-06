#include <alloc.h>
#include <string.h>
#include <estack.h>
#include <vat.h>

#include "secret_file.h"
#include "util.c"

const char FILE_TYPE_TAG [] = {0, 'S', 'E', 'C', 'R', 0, OTH_TAG};
const char secrets_path [] = "secrets\\";
const char SPECIAL_RESERVED_NAME [] = "manifest";

static int is_valid_name (char *name) {
	unsigned short i;
	for (i = 0; i < 8; i++) {
		if (name [i] == '\0')
			return i != 0;
		if (name [i] < 'a' || name [i] > 'z')
			return 0;
	}
	return 0;
}

/* These function take the secret file name as an zero terminated
 * ASCII string and a pointer to a loaded_secret.
 * they could return any one of the return codes defined in util.h */

/* before calling either of these functions, make sure to call
 * create_directory of util.h and that it is successful */

/* Just so you know, accessing files on the calculator's file system
 * is a highly bizarre process consisting of many steps,
 * involving many system calls, of which some are buggy, and if one
 * gets a single step wrong, they risk crashing the entire os. This
 * crash could happen even after the user program terminates.
 * It took many long hours to debug just these two functions */

static int write_secret (struct loaded_secret *sec, char *secret_file_name) {
	char file_name_buffer [9];
	char *file_sym_str_p = convert_to_sym_str (file_name_buffer, secret_file_name);
	/* check for reserved names */
	if (CheckReservedName(file_sym_str_p))
		return SECRET_RESERVED_NAME;
	if (strncmp (secret_file_name, SPECIAL_RESERVED_NAME, 8) == 0)
		return SECRET_RESERVED_NAME;
	if (!is_valid_name (secret_file_name))
		return SECRET_RESERVED_NAME;
	/* check for file already existing */
	HSym hs = FindSymInFolder (file_sym_str_p, DEFAULT_DIRECTORY_P);
	if (hs.folder != 0)
		return SECRET_ALREADY_EXISTS;
	/* length of the file, not inclusive of the first 2 bytes
	 * which specify the length of the rest of the file */
	unsigned short calculated_length = sec->secret_len + sec->label_len + 6 + sizeof (FILE_TYPE_TAG);
	/* allocate space for the file,
	 * inclusive of the 2 bytes for the length of the file */
	HANDLE h = HeapAlloc (calculated_length + 2);
	if (h == H_NULL)
		return SECRET_MEM;
	/* write the data to the file */
	MULTI_EXPR *calc_var = HeapDeref (h);
	if (calc_var == NULL)
		return SECRET_MEM;
	unsigned char *data = calc_var->Expr;
	*data = (unsigned char) sec->secret_len;
	data += 1;
	char *secret = data;
	memcpy (data, sec->secret, sec->secret_len);
	data += sec->secret_len;
	*data = (unsigned char) sec->label_len;
	data += 1;
	char *label = data;
	memcpy (data, sec->label, sec->label_len);
	data += sec->label_len;
	*data = '\0';
	data += 1;
	*data = sec->code_len;
	data += 1;
	memcpy (data, &sec->period, 2);
	data += 2;
	memcpy (data, FILE_TYPE_TAG, sizeof (FILE_TYPE_TAG));
	/* set the size of the file, not inclusive of the first 2 bytes */
	calc_var->Size = calculated_length;
	/* set the name for the variable entry 
	 * this somewhat-complex code is necessary because the rom call
	 * AddSymToFolder has bugs that lead to the variables being
	 * added to the wrong directory. we have to concatenate
	 * the directory and file name to pass directly to SymAdd
	 */
	char full_path [20], full_path_buf [20];
	strncpy (full_path, secrets_path, 20);
	strncat (full_path, secret_file_name, 20);
	char *full_path_p = convert_to_sym_str (full_path_buf, full_path);
	/* create the variable entry */
	hs = SymAdd(full_path_p);
	if (hs.folder == 0)
		goto error_with_memory_allocated;
	SYM_ENTRY *se = DerefSym (hs);
	if (se == NULL)
		goto error_with_file_created;
	/* assign the written data to the variable entry */
	se->handle = h;
	/* update the contents of the loaded secret structure */
	sec->file_handle = h;
	sec->secret = secret;
	sec->label = label;
	sec->archived = 0;
	return SECRET_OK;
	error_with_file_created:
	HSymDel (hs);
	error_with_memory_allocated:
	HeapFree (h);
	return SECRET_OTHER_ERROR;
}

static int read_secret (struct loaded_secret *sec, char *secret_file_name) {
	char file_name_buffer [9], full_path_buffer [20], full_path [20];
	char *file_sym_str_p = convert_to_sym_str (file_name_buffer, secret_file_name);
	/* check for reserved names */
	if (CheckReservedName(file_sym_str_p))
		return SECRET_RESERVED_NAME;
	if (!is_valid_name (secret_file_name))
		return SECRET_RESERVED_NAME;
	/* get file entry */
	strncpy (full_path, secrets_path, 20);
	strncat (full_path, secret_file_name, 20);
	char *full_path_p = convert_to_sym_str (full_path_buffer, full_path);
	HSym hs = SymFind (full_path_p);
	if (hs.folder == 0)
		return SECRET_FILE_NOT_FOUND;
	SYM_ENTRY *se = DerefSym (hs);
	if (se == NULL)
		return SECRET_OTHER_ERROR;
	if (se->flags.bits.folder)
		return SECRET_FILE_INVALID;
	sec->archived = se->flags.bits.archived == 1;
	MULTI_EXPR *calc_var = HeapDeref (se->handle);
	if (calc_var == NULL)
		return SECRET_OTHER_ERROR;
	/* The file size at the beginning is used to determine the size
	 * of the file instead of the size of the allocated space, but
	 * the size of the allocated space still has to be greater or equal
	 * to the stated size (plus 2 bytes for the size itself)
	 */
	if ((calc_var->Size + 2) > HeapSize (se->handle))
		return SECRET_FILE_INVALID;
	unsigned short n_bytes = 0;
	unsigned char *data = calc_var->Expr;
	/* read data, vaildate data */
	if (calc_var->Size == 0)
		return SECRET_FILE_INVALID;
	sec->secret_len = data [n_bytes];
	if (sec->secret_len == 0)
		return SECRET_FILE_INVALID;
	n_bytes += 1;
	if (calc_var->Size < (n_bytes + sec->secret_len))
		return SECRET_FILE_INVALID;
	sec->secret = &data [n_bytes];
	n_bytes += sec->secret_len;
	if (calc_var->Size < (n_bytes + 1))
		return SECRET_FILE_INVALID;
	sec->label_len = data [n_bytes];
	if (sec->label_len < 1)
		return SECRET_FILE_INVALID;
	n_bytes += 1;
	if (calc_var->Size < (n_bytes + sec->label_len + 1))
		return SECRET_FILE_INVALID;
	sec->label = &data [n_bytes];
	if (sec->label [sec->label_len] != '\0')
		return SECRET_FILE_INVALID;
	n_bytes += sec->label_len + 1;
	if (calc_var->Size < (n_bytes + 3))
		return SECRET_FILE_INVALID;
	sec->code_len = data [n_bytes];
	n_bytes += 1;
	memcpy (&(sec->period), &data [n_bytes], 2);
	if (sec->period == 0)
		return SECRET_FILE_INVALID;
	sec->file_handle = se->handle;
	return SECRET_OK;
}
