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

#endif
