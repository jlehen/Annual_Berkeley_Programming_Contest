#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned long long myint_t;

#define	UINT_SIZE	(sizeof (myint_t) * 8)
//#define	UINT_SIZE	16

static myint_t N, n1bits;

myint_t
count_n1bits(myint_t n)
{
	myint_t n1bits, i;

	for (n1bits = 0, i = 0; i < UINT_SIZE; i++) {
		n1bits += n & 0x1;
		n >>= 1;
	}
	return n1bits;
}

void
generate_n1bits_numbers(myint_t minrank, myint_t maxrank, myint_t otherbits)
{
        static myint_t position;
        myint_t mybit = 1 << minrank;
        myint_t generatednum;

        for (myint_t i = minrank; i <= maxrank; i++) {
                //printf("DEBUG: (min %u, max %u) - %u\n", minrank, maxrank, i);
                generatednum = mybit | otherbits;
                if (minrank == 0) {
                        if (generatednum == N) {
                            printf("%llu is number %llu in the sequence of "
                                "numbers with %llu 1-bits.\n",
                                N, position, n1bits);
                            exit(0);

                        }
                        /*
			printf("DEBUG: generatednum: %#lx/%u (%u 1-bits)\n",
                            generatednum, generatednum,
                            count_n1bits(generatednum));
                        */
                        position++;
                } else {
                        generate_n1bits_numbers(minrank - 1, i - 1,
                            otherbits | mybit);
                }
                mybit <<= 1;
        }
}

int
main(int argc, char *argv[])
{
	N = strtoul(argv[1], NULL, 10);
	n1bits = count_n1bits(N);
	printf("DEBUG: %llu 1-bits\n", n1bits);
	generate_n1bits_numbers(n1bits - 1, UINT_SIZE - 1, 0);
}

//287932420934278293879239832498237983273
