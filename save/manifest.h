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

#endif
