#ifndef TIME_ZONE_H
#define TIME_ZONE_H
#include <alloc.h>

#define TZ_OK 1
#define TZ_CANCELLED 2
#define TZ_MEM 2
#define TZ_ERROR 3

/* dialog for setting the time zone */
int run_time_zone_dialog (HANDLE manifest_handle, int wide_format);
#endif
