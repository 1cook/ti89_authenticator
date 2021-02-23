#ifndef HOTP_H
#define HOTP_H

#include <inttypes.h>

#include "../hash/algo.h"

/* HOTP as defined in RFC 4226 */
void hotp (
	/* output of hotp. null terminated string with code.
	 * allocated length must be code_len + 1 */
    char *dst,
	/* secret key */
	unsigned char *key,
	unsigned int key_len,
	/* counter */
	int64_t counter,
	/* number of decimal digits for the code */
	unsigned int code_len
);

#endif
