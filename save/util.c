#include "util.h"
#include <alloc.h>
#include <vat.h>

const char DEFAULT_DIRECTORY [] = "\0secrets";
const char *DEFAULT_DIRECTORY_P = DEFAULT_DIRECTORY + sizeof(DEFAULT_DIRECTORY) - 1;

/* Convert a zero terminated ASCII string into the ridiculous format
 * that the calculator insists on for its VAT handling functions.
 * pass into src the NULL terminated string of at most 8 characters
 * pass into buf, a buffer of 9 characters. this will not be passed
 * into any files, but it will be used as the storage space that backs
 * the pointer.
 * it returns the pointer that you should give to the VAT functions
 */
static char *convert_to_sym_str (char *buf, char *src) {
	char *dst = buf;
	*dst = '\0';
	dst += 1;
	while (*src != '\0') {
			*dst = *src;
			dst += 1;
			src += 1;
	}
	*dst = '\0';
	return dst;
}

/*
 * This macro is intended to be used for constant zero terminated 
 * strings that already begin with a null terminating byte.
 */

/* creates the secrets directory, if it does not exist
 * returns SECRET_OK normally, or with some other return
 * code from the above if not successful. */
static int create_directory () {
	short s = FolderFind (DEFAULT_DIRECTORY_P);
	if (s == NOT_FOUND) {
		HANDLE hh = FolderAdd (DEFAULT_DIRECTORY_P);
		if (hh == H_NULL)
			return SECRET_MEM;
	} else if (s == BAD_FOLDER || s == MAIN_FOLDER) /* Neither of these should ever happen */
		return SECRET_OTHER_ERROR;
	return SECRET_OK;
}
