#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// In 1991, I'm not sure err(3) did exist.
void
showerr(const char *fname, int line)
{
        fprintf(stderr, "%s@%d: %s\n", fname, line, strerror(errno));
        exit(1);
}

#define CHECK_ERR(fname, errval, val) \
    do { \
            if (val == errval) \
                    showerr(fname, __LINE__); \
    } while (0)


int fd;

// Each bit reprensent the primality of the natural number corresponding to
// the bit position.  Number 2 in stored in position 0.
typedef unsigned long storage_t;

#define MAX_PRIMES      3500000
#define BLOCK_SIZE      (sizeof (storage_t) * 8)                // in bits
#define BLOCK_COUNT     (MAX_PRIMES / BLOCK_SIZE)
storage_t *prime_list;

#define STORAGE_SIZE    (BLOCK_COUNT * sizeof (storage_t))      // in bytes
#define SEGMENT_SIZE    (50 * 4096)                             // 200 kB
#define BLOCKS_PER_SEG  (SEGMENT_SIZE / sizeof (storage_t))

#define BLOCK(id)       (prime_list[(id) % BLOCKS_PER_SEG])

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

// Debugging only.
void
print_block(storage_t block)
{
        storage_t i;

        for (i = 0; i < BLOCK_SIZE; i++)
                printf("%d", (block >> i) & 0x1UL);
        printf("\n");
}

// Maps the segment containing the block id, which need to be accessed modulo
// BLOCKS_PER_SEG.
void
remap_segment(storage_t blockid)
{
        static off_t previous_offset = -1;
        storage_t segment;
        off_t offset;
        int flags = MAP_SHARED;

        segment = blockid / BLOCKS_PER_SEG;
        offset = segment * SEGMENT_SIZE;

        if (prime_list != NULL && offset == previous_offset)
                return;
        previous_offset = offset;
        if (prime_list != NULL)
                flags |= MAP_FIXED;
	CHECK_ERR("mmap", MAP_FAILED, mmap(prime_list, SEGMENT_SIZE,
            PROT_READ | PROT_WRITE, flags, fd, offset));
}

void
cross_multiples(storage_t prime)
{
        storage_t multiple, blockid;

#define CHARIDX(n)      ((n - 2UL) / BLOCK_SIZE)
#define BITIDX(n)       ((n - 2UL) % BLOCK_SIZE)

	//printf("DEBUG: crossing multiples of %lu\n", prime);
        for (multiple = prime * prime;
            multiple < MAX_PRIMES; multiple += prime) {
                blockid = CHARIDX(multiple);
                remap_segment(blockid);

                BLOCK(blockid) |= (0x1UL << BITIDX(multiple));
        }
}

int
main(int argc, char *argv[])
{
        storage_t K;
	storage_t limit, blockno, number, i, count;
	char tempfile[] = "/tmp/91_4_2.XXXXXX";

	CHECK_ERR("mkstemp", -1, (fd = mkstemp(tempfile)));
	// Ensure the file has the write size.  Sparse files are filled with 0.
	CHECK_ERR("lseek", -1, lseek(fd, STORAGE_SIZE - 1, SEEK_SET));
	CHECK_ERR("write", -1, write(fd, "\0", 1));
        // Map the file once so prime_list is attributed an address.
	prime_list = mmap(NULL, SEGMENT_SIZE, PROT_READ | PROT_WRITE,
	    MAP_SHARED, fd, 0);
        CHECK_ERR("mmap", MAP_FAILED, prime_list);

        K = strtoul(argv[1], NULL, 10);
	limit = intsqrt(MAX_PRIMES, MAX_PRIMES);

        for (blockno = 0; blockno < BLOCK_COUNT; blockno++) {
                //printf("DEBUG: block %lu\n", blockno);
                //print_block(prime_list[blockno]);
		number = blockno * BLOCK_SIZE + 2;
		if (number > limit)
			break;
		for (i = 0; i < BLOCK_SIZE; i++) {
                        remap_segment(i);
			if (!((BLOCK(blockno) >> i) & 0x1UL))
				cross_multiples(number + i);
                }
        }

        count = 0;
        for (blockno = 0; blockno < BLOCK_COUNT; blockno++) {
		number = blockno * BLOCK_SIZE + 2;
		for (i = 0; i < BLOCK_SIZE; i++) {
                        remap_segment(i);
			if (!((BLOCK(blockno) >> i) & 0x1UL)) {
                                if (++count == K) {
                                        printf("Prime %lu is %lu\n",
                                            K, number + i);
                                        return 0;
                                }
                        }
                }
        }
}
