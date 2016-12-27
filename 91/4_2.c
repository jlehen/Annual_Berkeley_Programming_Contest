#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Each bit reprensent the primality of the natural number corresponding to
// the bit position.  Number 2 in stored in position 0.
typedef unsigned long storage_t;

#define MAX_PRIMES      3500000
#define BLOCK_SIZE      (sizeof (storage_t) * 8)
#define STORAGE_SIZE    (MAX_PRIMES / BLOCK_SIZE)
storage_t prime_list[STORAGE_SIZE];

// This is a poor man's version of Newton's method to compute a square root,
// using integer division, which can result in a period-two cycle sequence in
// some conditions.  So tweak the end condition a bit; this is incorrect but we
// only use this value to get an upper bound.
storage_t
intsqrt(storage_t n, storage_t xk)
{
        storage_t xk1;

        xk1 = (xk + n / xk) / 2;
	if (xk1 > xk) {
		if (xk1 - xk < 2)
			return xk1;
	} else {
		if (xk - xk1 < 2)
			return xk1;
	}
        intsqrt(n, xk1);
}

void
print_block(storage_t block)
{
        storage_t i;

        for (i = 0; i < BLOCK_SIZE; i++)
                printf("%d", (block >> i) & 0x1UL);
        printf("\n");
}

void
cross_multiples(storage_t prime)
{
        storage_t multiple;

#define CHARIDX(n)      ((n - 2UL) / BLOCK_SIZE)
#define BITIDX(n)       ((n - 2UL) % BLOCK_SIZE)

	//printf("DEBUG: crossing multiples of %lu\n", prime);
        for (multiple = prime * prime; multiple < MAX_PRIMES; multiple += prime) {
                prime_list[CHARIDX(multiple)] |= (0x1UL << BITIDX(multiple));
        }
}

int
main(int argc, char *argv[])
{
        storage_t K;
	storage_t limit, blockno, number, i, count;

        K = strtoul(argv[1], NULL, 10);
	limit = intsqrt(MAX_PRIMES, MAX_PRIMES);

        for (blockno = 0; blockno < STORAGE_SIZE; blockno++) {
                //printf("DEBUG: block %lu\n", blockno);
                //print_block(prime_list[blockno]);
		number = blockno * BLOCK_SIZE + 2;
		if (number > limit)
			break;
		for (i = 0; i < BLOCK_SIZE; i++) {
			if (!((prime_list[blockno] >> i) & 0x1UL))
				cross_multiples(number + i);
                }
        }

        count = 0;
        for (blockno = 0; blockno < STORAGE_SIZE; blockno++) {
		number = blockno * BLOCK_SIZE + 2;
		for (i = 0; i < BLOCK_SIZE; i++)
			if (!((prime_list[blockno] >> i) & 0x1UL)) {
                                if (++count == K) {
                                        printf("Prime %lu is %lu\n",
                                            K, number + i);
                                        return 0;
                                }
                        }
        }
}
