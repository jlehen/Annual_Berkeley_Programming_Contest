#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PRIMES  250000
#define NPAGES      50
#define SLABSIZE    (PAGE_SIZE * NPAGES)      // 200 kB
#define PRIMES_PER_SLAB   (SLABSIZE / sizeof (unsigned))

static unsigned *slab;          // allocated with mmap(2)
static unsigned upperbound;     // current slab upper index
static unsigned idx = 0;        // 0 <= idx < primecount
unsigned primecount;            // total number of primes
int fd;

// In 1991, I'm not sure err(3) did exist.
void
showerr(const char *fname, int line)
{
        fprintf(stderr, "%s@%d: %s\n", fname, line, strerror(errno));
        exit(1);
}

#define CHECKERR(fname, errval, val) \
    do { \
            if (val == errval) \
                    showerr(fname, __LINE__); \
    } while (0)

// This is a poor man's version of Newton's method to compute a square root,
// using integer division, which can result in a period-two cycle sequence in
// some conditions.  So tweak the end condition a bit; this is incorrect but we
// only use this value to get an upper bound.
unsigned
intsqrt(unsigned n, unsigned xk)
{
        unsigned xk1;

        xk1 = (xk + n / xk) / 2;
        if (xk1 - xk < 2)
                return xk1;
        intsqrt(n, xk1);
}

void
slabremap(off_t offset)
{
        static off_t previous_offset = -1;
        int flags = MAP_SHARED;

        if (slab != NULL && offset == previous_offset)
                return;
        //printf("DEBUG: remap slab to offset %u\n", offset);
        if (slab != NULL) {
                //CHECKERR("lseek", -1, lseek(fd, previous_offset, SEEK_SET));
                //CHECKERR("write", -1, write(fd, slab, SLABSIZE));
                flags |= MAP_FIXED;
        }
        previous_offset = offset;
        //CHECKERR("lseek", -1, lseek(fd, offset, SEEK_SET));
        //CHECKERR("read", -1, read(fd, slab, SLABSIZE));
        CHECKERR("mmap", MAP_FAILED, mmap(slab, SLABSIZE,
            PROT_READ | PROT_WRITE, flags, fd, offset));
}

unsigned
nextprime(unsigned current)
{
        static unsigned previous;
        unsigned prime;
        unsigned limit = intsqrt(current, current);

        if (previous != current) {
                //printf("DEBUG %u: new number, previous %u\n", current, previous);
                previous = current;
                idx = 0;
        }
        if (idx == 0) {
                slabremap(0);
                upperbound = PRIMES_PER_SLAB;
        }
        if (idx == upperbound) {
                slabremap(upperbound * sizeof (unsigned));
                upperbound += PRIMES_PER_SLAB;
        }
        if (idx == primecount) {
                //printf("DEBUG %u: exhausted available primes\n", current);
                idx = 0;
                return 0;
        }
        prime = slab[idx % PRIMES_PER_SLAB];
        if (prime > limit) {
                //printf("DEBUG %u: %uth prime %u above limit %u\n", current, idx, prime, limit);
                idx = 0;        // start over
                return 0;
        }
        idx++;
        //printf("DEBUG %u: %uth prime is %u\n", current, idx, prime);
        return prime;
}

void
addprime(unsigned prime)
{
        slabremap((primecount / PRIMES_PER_SLAB) *
            PRIMES_PER_SLAB * sizeof (unsigned));
        slab[primecount % PRIMES_PER_SLAB] = prime;
        primecount++;
}

int
main(int argc, char *argv[])
{
        char filename[] = "/tmp/91_4.XXXXXXX";
        unsigned K, current, lastprime;

        fd = mkstemp(filename);
        CHECKERR("mkstemp", -1, fd);
        // Ensure the file is big enough to be mapped.
        CHECKERR("lseek", -1, lseek(fd, MAX_PRIMES * sizeof (unsigned) - 1,
            SEEK_SET));
        CHECKERR("write", -1, write(fd, "\0", 1));
        CHECKERR("lseek", -1, lseek(fd, 0, SEEK_SET));
        // Ensure slab has a valid address.
        slab = mmap(NULL, SLABSIZE, PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANON, -1, 0);

        K = strtoul(argv[1], NULL, 10);
        // Optimization to run only on odd numbers.
        addprime(2);
        lastprime = 2;
        for (current = 3; primecount != K; current += 2) {
                int isprime = 1;
                while (1) {
                        unsigned prime = nextprime(current);
                        if (prime == 0)
                                break;
                        if (current % prime == 0) {
                                isprime = 0;
                                break;
                        }
                }
                if (isprime) {
                        //printf("DEBUG: %uTH PRIME!  %u\n", primecount, current);
                        addprime(current);
                        lastprime = current;
                }
        }
        printf("Prime %u is %u\n", K, lastprime);
}
