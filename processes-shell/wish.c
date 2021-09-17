#include "wish.h"

// Error message
void __error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

/*
 * Built-in exit method
 */
void __exit() {
    exit(0);
}

/*
 * Built-in cd method: change directory
 */
void __cd(const char *arg) {
    // ref: chdir
}

/*
 * Built-in path method: add paths as search paths to the shell
 * It will overwrite the old path with the newly specified path.
 * 
 * e.g.
 * path /bin /usr/bin
 */
void __path(void *argv[]) {

}

/*
 * Execute the command
 * // TODO: support absolute and relative paths
 */
int execute_command(void *args) {
    // Find executables in pre-defined paths
    // ref: access()
    return 0;
}

/*
 * Display output to output stream
 */
int display_output(char *output, FILE *file) {
    int rc = 0;
    rc = fwrite(output, sizeof(char), strlen(output), file);
    return rc;
}

/*
 * Execute the commands input by the stream
 */
int execute_commands(FILE *file) {
    char *line = NULL;
    char *output = NULL;
    size_t len = 0;
    int nread = 0;
    FILE *ostream = NULL;
    while (1) {
        // Prompt
        printf("%s", PROMPT);

        // Get the line
        // ref: getline()
        nread = getline(&line, &len, file);

        // Exit when entering exit
        // or hit EOF
        // TODO: other error
        if (nread == -1) break;

        // Parse the line
        // ref: strsep()

        // Execute command in new processes (including parallel situation),
        // and output them one by one either to stdout or file
        // ref: fork(), execv(), waitpid()
        // e.g. wish> ls -la /tmp > output
        // e.g. wish> cmd1 & cmd2 args1 args2 & cmd3 args1
        ostream = stdout;
        // for (int i = 0; i < cmd_size; i++) {
        //     output = execute_command(argv[i]);
        //     display_output(output, ostream);
        // }
        output = line;
        display_output(output, ostream);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    int rc = 0;
    FILE *istream;

    // Iteractive mode
    // File stream is stdin
    // (stdin is a stream, while STDIN_FILENO is a file descriptor)
    // ref: 
    // 1. https://stackoverflow.com/questions/22367920/is-it-possible-that-linux-file-descriptor-0-1-2-not-for-stdin-stdout-and-stderr
    // 2. https://stackoverflow.com/questions/44423865/what-exactly-is-stdin
    if (argc == 1) {
        istream = stdin;
    }

    // Batch mode
    // File stream is the batch file
    else if (argc == 2) {
        istream = fopen(argv[1], "r");
        if (istream == NULL) {
            perror("Fail to open the file!\n");
            exit(1);
        }
    }

    // Invalid arguments
    else {
        printf("wish shell can only take one or two arguments!\n");
        exit(1);
    }

    // Execute command input by the stream
    rc = execute_commands(istream);
    return rc;
}