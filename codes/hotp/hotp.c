#include <stdio.h>
#include <inttypes.h>
#include "hotp.h"
#include "../hmac/hmac.h"

/* HOTP as defined in RFC 4226 */
static void hotp (
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
) {
	/* Do Hmac with specified hash function */
	unsigned char hmac_output [20];
	hmac (hmac_output, key, key_len, (unsigned char *) &counter, 8);
	/* do truncation to single integer */
	unsigned int offset = hmac_output [19] & 0xf;
	uint32_t code_numeric = ((uint32_t) hmac_output [offset] & 0x7f) << 24
		| ((uint32_t) hmac_output [offset + 1]) << 16
		| ((uint32_t) hmac_output [offset + 2]) << 8
		| ((uint32_t) hmac_output [offset + 3]);
	/* shorten to code length, convert to ascii decimal */
	uint32_t mod = 1;
	unsigned int i = 0;
	for (i = 0; i < code_len; i++)
		mod *= 10;
	code_numeric %= mod;
	switch (code_len) {
		case 6:
			sprintf (dst, "%06lu", code_numeric);
			break;
		case 7:
			sprintf (dst, "%07lu", code_numeric);
			break;
		case 8:
			sprintf (dst, "%08lu", code_numeric);
			break;
		case 9:
			sprintf (dst, "%09lu", code_numeric);
			break;
		case 10:
			sprintf (dst, "%010lu", code_numeric);
			break;
	}
}
