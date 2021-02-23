#ifndef CALC_HASH_ALGO_H
#define CALC_HASH_ALGO_H

/*
 * Description of a hashing algorithm.
 */
struct hash_algo_desc 
{
	/* output length of hash */
	unsigned int output_len;
	/* length of hash functions internal state */
	unsigned int ctx_len;
	/* pointer to hashing function initialization */
	void (*init) (
		/* 
		 * containing pointer to memory with internal state
		 * must be at least ctx_len long in memory
		 */
		void *ctx
	);
	/* pointer to function to update hash with new data */
	void (*update) (
		void *ctx,
		unsigned char *data,
		unsigned int data_len
	);
	/* pointer to function to complete and output hash */
	void (*finalize) (
		unsigned char *dst,
		void *ctx
	);
	char name[];
};

#endif
