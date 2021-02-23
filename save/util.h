#ifndef SAVE_UTIL_H
#define SAVE_UTIL_H

/* constants for return codes */
#define SECRET_OK 1
#define SECRET_OTHER_ERROR 2
#define SECRET_FILE_NOT_FOUND 3
#define SECRET_FILE_INVALID 4
#define SECRET_ALREADY_EXISTS 5
#define SECRET_MEM 6
#define SECRET_RESERVED_NAME 7

/* the name of the default directory */
const char *DEFAULT_DIRECTORY_P;

/* Convert a zero terminated ASCII string into the ridiculous format
 * that the calculator insists on for its VAT handling functions.
 * pass into src the NULL terminated string of at most 8 characters
 * pass into buf, a buffer of 9 characters. this will not be passed
 * into any files, but it will be used as the storage space that backs
 * the pointer.
 * it returns the pointer that you should give to the VAT functions
 */
char *convert_to_sym_str (char *buf, char *src);

/*
 * This macro is intended to be used for constant zero terminated 
 * strings that already begin with a null terminating byte.
 */

/* creates the secrets directory, if it does not exist
 * returns SECRET_OK normally, or with some other return
 * code from the above if not successful. */
int create_directory ();

#endif
