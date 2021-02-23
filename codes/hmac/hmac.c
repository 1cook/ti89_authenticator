#include "hmac.h"
#include <string.h>
#include <stdlib.h>

#include "../hash/sha1/sha1/sha1.h"

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

void hmac (
	unsigned char *dst,	
	unsigned char *key,
	unsigned int key_len,
	unsigned char *data,
	unsigned int data_len
)
{
	unsigned int i;
	unsigned char k_inner [64];
	unsigned char k_outer [64];
	
	/* prepare inner and houter hashes */
	memset (k_inner, 0, 64UL);
	memset (k_outer, 0, 64UL);
	memcpy (k_inner, key, (unsigned long) key_len);
	memcpy (k_outer, key, (unsigned long) key_len);
	
	for (i = 0; i < 64; i++) {
		k_inner [i] ^= HMAC_IPAD;
		k_outer [i] ^= HMAC_OPAD;
	}
	
	SHA1_CTX ctx;
	/* do inner hash */
	SHA1Init (&ctx);
	SHA1Update (&ctx, k_inner, 64UL);
	SHA1Update (&ctx, data, (uint32_t) data_len);
	SHA1Final (dst, &ctx);
	/* do outer hash */
	SHA1Init (&ctx);
	SHA1Update (&ctx, k_outer, 64UL);
	SHA1Update (&ctx, dst, 20UL);
	SHA1Final (dst, &ctx);
}
