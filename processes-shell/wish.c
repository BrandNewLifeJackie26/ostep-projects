#include "wish.h"

char *paths[MAX_PATH_COUNT];
int path_count = 0;

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
 * param: 
 *  path: directory to change to
 * 
 * ref: chdir()
 * 
 * e.g.
 * cd /bin
 */
void __cd(const char *path) {
    if (chdir(path) == -1) __error();
}

/*
 * Built-in path method: add paths as search paths to the shell
 * It will overwrite the old path with the newly specified path.
 * 
 * e.g.
 * path /bin /usr/bin -> resulting paths ['/bin', '/usr/bin']
 * path -> resulting paths []
 */
void __path(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        paths[i] = argv[i];
    }

    // All paths after argc will be omitted
    path_count = argc;
}

/*
 * Execute the command.
 * 
 * ostream: output stream (stdout or a specific file)
 * argc: argument number
 * argv[0]: executable or built-in
 * argv[1] ~ argv[argc-1]: arguments
 * 
 * // TODO: support absolute and relative paths
 */
int execute_command(FILE *ostream, int argc, char *argv[]) {
    int rc = 0;

    if (argc < 1) {
        __error();
        return -1;
    }
    char *command_name = argv[0];

    // Built-in methods
    if (strcmp(command_name, EXIT_COMMAND) == 0) {
        if (argc > 1) __error();
        else __exit();
    }        
    else if (strcmp(command_name, CD_COMMAND) == 0) {
        if (argc > 2) __error();
        else __cd(argv[1]);
    } else if (strcmp(command_name, PATH_COMMAND) == 0) {
        __path(argc-1, argv+1);
    }

    // Executables
    else {
        char executable[MAX_EXECUTABLE_COUNT];
        int pid = 0;
        int executable_found = 0;
        for (int i = 0; i < path_count; i++) {
            strcpy(executable, paths[i]);
            strcat(executable, "/");
            strcat(executable, command_name);

            // Execute the accessible executable
            // Find executables in pre-defined paths
            // ref: access()
            if (access(executable, X_OK) == 0) {
                executable_found = 1;
                pid = fork();

                // fork() fails
                if (pid == -1) {
                    __error();
                }
                
                // execv() in child process
                else if (pid == 0) {
                    // BUG: ls . -> cannot access ''?
                    if (execv(executable, argv) == -1) {
                        __error();
                        exit(1);
                    }
                } else {
                    // parent process will wait until child finishes
                    if (waitpid(pid, NULL, 0) == -1) {
                        __error();
                    }
                }
                break;
            }
        }

        // Cannot find the executable
        if (executable_found == 0) __error();
    }
    
    // Display output if needed
    // Redirect by overwriting stdout
    // for (int i = 0; i < argc; i++) {
    //     rc = fwrite(argv[i], sizeof(char), strlen(argv[i]), ostream);
    //     char enter = '\n';
    //     rc = fwrite(&enter, sizeof(char), 1, ostream);
    // }
    return rc;
}

/*
 * Execute the commands input by the stream
 */
int execute_commands(FILE *file) {
    char *line = NULL;
    size_t len = 0;
    int nread = 0;
    int rc = 0;
    FILE *ostream = NULL;

    // Command arguments init
    char *arg = NULL;
    char **argv = NULL;
    int argc = 0;

    while (1) {
        // Prompt
        if (file == stdin) printf("%s", PROMPT);

        // Get the line
        // ref: getline()
        nread = getline(&line, &len, file);

        // Exit when entering exit
        // or hit EOF
        // TODO: other error
        if (nread == -1) break;

        // Parse the line
        // TODO: redirect
        // ref: strsep()
        arg = line;
        argc = 0;
        argv = (char **)malloc(MAX_ARGC * sizeof(char *));
        if (argv == NULL) {
            __error();
            exit(1);
        }
        while ((arg = strsep(&line, DELIMETER)) != NULL && *arg != '\0') {
            argv[argc] = arg;
            argc++;
        }

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

        // Single command
        rc = execute_command(ostream, argc, argv);
        
        // Free space
        free(argv);
    }

    return rc;
}

int main(int argc, char const *argv[])
{
    int rc = 0;
    FILE *istream;

    // Initialize paths
    paths[0] = "/bin";
    path_count = 1;

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