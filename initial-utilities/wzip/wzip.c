#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    FILE *file = NULL;
    char prev = EOF, curr = EOF;
    int count = 0;
    for (int i = 1; i < argc; i++) {
        file = fopen(argv[i], "r");

        // Iterate over the file and count
        while ((curr = fgetc(file)) != EOF) {
            // New char
            if (curr != prev || count == 0) {
                if (count != 0) {
                    fwrite(&count, 4, 1, stdout);
                    fwrite(&prev, 1, 1, stdout);
                }
                
                count = 1;
                prev = curr;
            }

            // Repeating chars
            else {
                count++;
            }
        }        
        if (fclose(file) == EOF) return 1;
    }

    // Last char
    if (count != 0) {
        fwrite(&count, 4, 1, stdout);
        fwrite(&prev, 1, 1, stdout);
    }

    return 0;
}
