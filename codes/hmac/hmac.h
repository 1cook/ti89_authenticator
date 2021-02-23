#ifndef CALC_HMAC_H
#define CALC_HMAC_H
#include "../hash/algo.h"

#include <inttypes.h>

#define ALG_SHA1 0

/* HMAC as defined in RFC 2104 */
void hmac (
	/* 
	 * pointer to output result
	 * assumed to be large enough for hash output
	 */
	unsigned char *dst,
	/* pointer to secret key, must be key_length bytes long */
	unsigned char *key,
	/* length of secret key. not longer than 64 bytes.
	 * if you have a longer key, then apply the hash function 
	 * to it before passing it into this function */
	unsigned int key_len,
	/* message to compute HMAC over */
	unsigned char *data,
	/* length of message to compute HMAC over */
	unsigned int data_len
);

#endif
