#ifndef SECRET_FILE_H
#define SECRET_FILE_H
#include "util.h"
#include <alloc.h>
#include <inttypes.h>

/* this represents a secret which has been loaded into memory
 * the below functions do not automatically perform memory management
 * on pointers to secret and label, however they may change
 * these pointers */
struct loaded_secret {
	/* handle of the loaded file */
	HANDLE file_handle;
	/* this is not zero terminated and not necessairly ASCII  */
	unsigned char *secret;
	unsigned int secret_len;
	/* this is zero terminated */
	char *label;
	unsigned int label_len;
	unsigned char code_len;
	/* a period of 0 is invalid */
	unsigned short period;
	unsigned char current_code [12];
	int64_t next_update_at;
};


/* These function take the secret file name as an zero terminated
 * ASCII string and a pointer to a loaded_secret.
 * they could return any one of the return codes defined in util.h*/
 
/* before calling either of these functions, make sure to call
 * create_directory of util.h and that it is successful */

int write_secret (struct loaded_secret *sec, char *secret_file_name);
int read_secret (struct loaded_secret *sec, char *secret_file_name);

#endif

/* Just so you know, accessing files on the calculator's file system
 * is a highly bizarre process consisting of many steps,
 * involving many system calls, of which some are buggy, and if one
 * gets a single step wrong, they risk crashing the entire os. This
 * crash could happen even after the user program terminates.
 * It took many long hours to debug just these two functions */
