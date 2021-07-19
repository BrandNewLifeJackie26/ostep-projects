#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    FILE *file = NULL;
    int count = 0;
    char c = EOF;
    for (int i = 1; i < argc; i++) {
        file = fopen(argv[i], "r");

        // Iterate over the file and get the char and count
        while (fread(&count, 4, 1, file) == 1) {
            fread(&c, 1, 1, file);
            for (int i = 0; i < count; i++) fwrite(&c, 1, 1, stdout);
        }

        if (fclose(file) == EOF) return 1;
    }

    return 0;
}
