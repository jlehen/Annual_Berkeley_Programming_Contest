#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIM   '&'

enum step {
    compute_columns_size = 0,
    print_columns = 1,
    laststep = 2
};

int
main(int argc, char *argv[])
{
        int ncol = 1;
        int *cols = NULL;
        enum step step = compute_columns_size;

        while (step < laststep) {
                FILE *f;
                char *line = NULL, *p;
                size_t linesz = 0;

                f = fopen(argv[1], "r");
                while (getline(&line, &linesz, f) != -1) {
                        if (cols == NULL) {
                                for (p = line; *p != '\0'; p++)
                                        if (*p == DELIM)
                                                ncol++;
                                cols = malloc(ncol * sizeof (cols[0]));
                                bzero(cols, ncol * sizeof (cols[0]));
                        }

                        p = line;
                        for (int i = 0; i < ncol; i++) {
                                char *tok = p;
                                while (*p != DELIM && *p != '\0')
                                        p++;
                                *p = '\0';

                                // Remove leading and trailing whitespaces;
                                while (isspace(*tok) && *tok != '\0')
                                        tok++;
                                if (tok < p) {      // handles empty fields
                                        char *backup = p;
                                        p--;
                                        while (isspace(*p) && p > tok)
                                                *p-- = '\0';
                                        p = backup;
                                }

                                int toksz = strlen(tok);
                                if (step == compute_columns_size) {
                                        if (toksz > cols[i])
                                                cols[i] = toksz;
                                } else if (step == print_columns) {
                                        int padding = (cols[i] - toksz) / 2;
                                        char *morepadding = "";
                                        if ((cols[i] - toksz) % 2 != 0)
                                                morepadding = " ";
                                        char *separator = "|";
                                        if (i == ncol - 1)
                                                separator = "";
                                        printf(" %s%*s%s%*s %s", morepadding,
                                                padding, "", tok, padding, "",
                                                separator);
                                }
                                tok = p++;
                        }
                        /*
                        if (step == compute_columns_size) {
                                for (int i = 0; i < ncol; i++) {
                                        printf("col %d: %d\n", i, cols[i]);
                        }
                        */
                        if (step == print_columns)
                                printf("\n");
                }

                step++;
                free(line);
                fclose(f);
        }
}
