#include <stdio.h>

#define LINE_MAX 4096

int main(int argc, char const *argv[])
{
    char line[LINE_MAX];

    for (int i = 1; i < argc; i++) {
        // Open the file in read-only mode
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            printf("wcat: cannot open file\n");
            return 1;
        }

        // Print to terminal
        while (fgets(line, LINE_MAX, file) != NULL) {
            printf("%s", line);
        }

        // Close the file stream
        if (fclose(file) == EOF) return 1;
    }
    return 0;
}
