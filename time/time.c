/* this implementation adapted from https://howardhinnant.github.io/date_algorithms.html#civil_from_days */
#include "time.h"
int64_t timestamp_from_civil (unsigned short year, unsigned short month, unsigned short day, unsigned short hour, unsigned short minute, unsigned short second) {
	year -= month <= 2;
	const int32_t era = (year >= 0 ? year : year - 399) / 400;
	uint64_t yoe = (uint32_t)(year - era * 400);
	uint64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
	uint64_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;
	int64_t total_days = era * 146097 + (int64_t)(doe) - 719468;

	return total_days * 86400 + ((int64_t) hour) * 3600 + ((int64_t) minute) * 60 + (int64_t) second;
}
