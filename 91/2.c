#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Exercise
//#define COMP <
// Palindrome
#define COMP !=

int
main(int argc, char *argv[])
{
        const char *dictfilename;
        FILE *dictfile;
        char word[128];
        int **wordmaps;
        int charmap[26];
        int i;
        char *cp;
        int match;

        dictfilename = argv[1];
        argv += 2;
        argc -= 2;
        wordmaps = malloc(argc * sizeof (int **));
        for (i = 0; i < argc; i++) {
                wordmaps[i] = malloc(26 * sizeof (charmap));
                memset(wordmaps[i], 0, 26 * sizeof (charmap));
                for (cp = argv[i]; *cp != '\0'; cp++)
                        wordmaps[i][toupper(*cp) - 'A']++;
                /*
                for (unsigned n = 0; n < 26; n++) {
                        printf("%c: %u\n", n + 'A', wordmaps[i][n]);
                }
                printf("\n");
                */
        }

        dictfile = fopen(dictfilename, "r");
        while (fgets(word, 128, dictfile) != NULL) {
                for (i = 0; i < argc; i++) {
                        memcpy(charmap, wordmaps[i], sizeof (charmap));
                        for (i = 0; word[i] != '\0'; i++)
                                if (isalpha(word[i]))
                                        charmap[toupper(word[i]) - 'A']--;
                        for (match = 1, i = 0; i < 26 && match; i++) {
                                if (charmap[i] COMP 0)
                                        match = 0;
                        }
                        if (match) {
                                printf("%s", word);
                                break;
                        }
                }
        }
}
