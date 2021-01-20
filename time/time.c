/* this implementation adapted from https://howardhinnant.github.io/date_algorithms.html#civil_from_days */
#include "time.h"

#include <stdio.h>
#include <statline.h>
#include <unknown.h>

const char *DOW [7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *MOY [12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int64_t get_timestamp (int set_stat, short time_zone) {
	unsigned short year, month, day, hour, minute, second;
	char statline_string [60];
	DateAndTime_Get (&year, &month, &day, &hour, &minute, &second);
	int64_t current_time = timestamp_from_civil (year, month, day, hour, minute, second);
	current_time -= time_zone * 60;
	
	if (set_stat) {
		unsigned short dow = DayOfTheWeek (year, month, day);
		sprintf(statline_string, "%02d %s %04d %s %02d:%02d:%02d", day, MOY [month - 1], year, DOW [dow - 1], hour, minute, second);
		ST_helpMsg (statline_string);
	}
	return current_time;
}

int64_t timestamp_from_civil (unsigned short year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second) {
	year -= month <= 2;
	const int32_t era = (year >= 0 ? year : year - 399) / 400;
	uint64_t yoe = (uint32_t)(year - era * 400);
	uint64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
	uint64_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;
	int64_t total_days = era * 146097 + (int64_t)(doe) - 719468;

	return total_days * 86400 + ((int64_t) hour) * 3600 + ((int64_t) minute) * 60 + (int64_t) second;
}
