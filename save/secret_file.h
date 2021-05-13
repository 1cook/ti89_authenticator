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
	int archived;
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


#endif
