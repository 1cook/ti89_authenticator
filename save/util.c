#include "util.h"
#include <alloc.h>
#include <vat.h>

const char DEFAULT_DIRECTORY [] = "\0secrets";
const char *DEFAULT_DIRECTORY_P = DEFAULT_DIRECTORY + sizeof(DEFAULT_DIRECTORY) - 1;

char *convert_to_sym_str (char *buf, char *src) {
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

int create_directory () {
	short s = FolderFind (DEFAULT_DIRECTORY_P);
	if (s == NOT_FOUND) {
		HANDLE hh = FolderAdd (DEFAULT_DIRECTORY_P);
		if (hh == H_NULL)
			return SECRET_MEM;
	} else if (s == BAD_FOLDER || s == MAIN_FOLDER) /* Neither of these should ever happen */
		return SECRET_OTHER_ERROR;
	return SECRET_OK;
}
