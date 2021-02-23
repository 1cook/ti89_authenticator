#ifndef TIME_H
#define TIME_H

#include <inttypes.h>

/*
 * TOTP very reasonably uses unix time,
 * but the calcualtors regretably have no concept of this.
 * They only have this system call called DateAndTime_Get
 * which gives calendar days.
 */
int64_t timestamp_from_civil (
	unsigned short year,
	unsigned short month,
	unsigned short day,
	unsigned short hour,
	unsigned short minute,
	unsigned short second
);
#endif

/* The calculator also has no concept of time-zones.
 * users can be expected to set the clock on their calcualtor
 * to their local time while the unix epoch and therefore TOTP
 * will be in UTC. You must do a conversion sometime before
 * calculating the TOTP period.
 */
