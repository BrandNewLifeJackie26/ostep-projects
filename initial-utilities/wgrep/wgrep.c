#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    // No command line arguments
    if (argc < 2) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }

    // Iterate over all files to search the given term
    char const *term = argv[1];
    char *line = NULL;
    size_t line_size = 0;

    // No file input, read stdin instead
    if (argc == 2) {
        FILE *in = stdin;

        // Search the file for the term
        line = NULL;
        while (getline(&line, &line_size, in) != -1) {
            // Search and print
            if (strstr(line, term) != NULL) printf("%s", line);
        }
    }

    // Read input files
    for (int i = 2; i < argc; i++) {
        // Open the file
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            printf("wgrep: cannot open file\n");
            return 1;
        }

        // Search the file for the term
        line = NULL;
        while (getline(&line, &line_size, file) != -1) {
            // Search and print
            if (strstr(line, term) != NULL) printf("%s", line);
        }

        // Close the file
        if (fclose(file) == EOF) return -1;
    }
    return 0;
}
