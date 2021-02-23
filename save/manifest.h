/* This is the manifest file. It contains a list of all tracked
 * secrets files as well as the current time zone. It provides the
 * ordering  */

#ifndef MANIFEST_H
#define MANIFEST_H

/* return codes */
#define MANIFEST_OK 1 
#define MANIFEST_NEWLY_CREATED 0
#define MANIFEST_MEM 2
#define MANIFEST_INVALID 3
#define MANIFEST_LOCKED 4
#define MANIFEST_OTHER_ERROR 5
#define MANIFEST_TOO_MANY_FILES 6
#define MANIFEST_DUPLICATE 7

#include <alloc.h>

/* call create_directory of util.h before calling any of these
 * below functions
 */

/* loads the manifest file, creates if it does not already exists
 * returns the handle to the file or H_NULL if unsuccessful
 * 
 * this does not guarantee that the files listed in the manifest
 * actually exist or are valid
 */
HANDLE init_manifest (int *error_code);
/* tz is the timezone utc offset expressed in minutes 
 * returns one of the error codes
 */
int set_time_zone (HANDLE manifest_handle, short tz);
/* like set_time_zone. tz will be filled with the current time zone
 */
int get_time_zone (HANDLE manifest_handle, short *tz);

/* these are just like set_time_zone and get_time_zone respectively */
int get_position (HANDLE manifest_handle, unsigned short *pos);
int set_position (HANDLE manifest_handle, unsigned short pos);

/* adds a filename onto the end of the manifest file */
int add_file (HANDLE manifest_handle, char *file_name);
int remove_file (HANDLE manifest_handle, unsigned short index);
/* gets a file name from the manifest file 
 * file name points to a buffer that is at least nine bytes long.
 * will write the name of the file to it, adding zero terminator.
 */
int get_file (HANDLE manifest_handle, char *file_name, unsigned short index);
/* swaps the positions of one file with another in the manifest */
int swap_positions (HANDLE manifest_handle, unsigned short pos1, unsigned short pos2);
unsigned short get_size (HANDLE manifest_handle);
#endif
