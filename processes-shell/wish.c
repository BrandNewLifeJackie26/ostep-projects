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
        strcpy(paths[i], argv[i]);
    }

    // All paths after argc will be omitted
    path_count = argc;
}

/*
 * Parse the command by tokenizing all arguments and get rid of all empty ones.
 *
 * arguments:
 *  command: the original command, will be altered
 *  argc: the final count of all arguments
 *  argv: the final list of all arguments
 * 
 * e.g.
 * ls /dir & cd & ls /dd > b.txt
 *  -> ["ls", "/dir", "&", "cd", "&", "ls", "/dd", ">", "b.txt"]
 */
void parse_command(char *command, int *argc, char **argv) {
    char *temp = command;
    char *arg;

    *argc = 0;
    while (temp != NULL) {
        // Find the token between current temp and the next delimiter,
        // and split the token, set temp to next char, and then return the original beginning
        arg = strsep(&temp, DELIMETER);

        // Empty token
        if (strcmp(arg, "") == 0) continue;

        argv[*argc] = arg;
        (*argc)++;
    }
}

/*
 * Execute the command.
 * 
 * filename: filename of the redirection file, NULL if output to stdout
 * argc: argument number
 * argv[0]: executable or built-in
 * argv[1] ~ argv[argc-1]: arguments
 * 
 * ref: fork(), execv(), waitpid()
 * e.g. wish> ls -la /tmp > output
 * e.g. wish> cmd1 & cmd2 args1 args2 & cmd3 args1
 * 
 * // TODO: support absolute and relative paths
 */
int execute_command(char *filename, int argc, char *argv[]) {
    int rc = 0;

    // Empty command
    if (argc < 1) {
        if (filename != NULL) __error();
        return 0;
    }
    char *command_name = argv[0];

    // Built-in methods
    if (strcmp(command_name, EXIT_COMMAND) == 0) {
        if (argc > 1) __error();
        else __exit();
    } else if (strcmp(command_name, CD_COMMAND) == 0) {
        if (argc > 2) __error();
        else __cd(argv[1]);
    } else if (strcmp(command_name, PATH_COMMAND) == 0) {
        __path(argc-1, argv+1);
    }

    // Executables
    else {
        char executable[MAX_EXECUTABLE_LENGTH];
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
                } else if (pid == 0) {
                    // Redirect to file
                    int fd;
                    if (filename != NULL) {
                        if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0xffff)) == -1 ||
                            dup2(STDOUT_FILENO, fd) == -1 ||
                            dup2(STDERR_FILENO, fd) == -1) {
                                __error();
                                return -1;
                            }
                    }

                    // execv() in child process
                    argv[argc] = NULL;
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

    // Command arguments init
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
        argc = 0;
        argv = (char **)malloc(MAX_ARGC * sizeof(char *));
        if (argv == NULL) {
            __error();
            exit(1);
        }

        // Split parallel commands
        // cmd refers to one command that is run concurrently with others
        // // TODO: declaration should be moved out of while loop
        char *cmds[MAX_CMD_COUNT];
        int cmd_count = 0;
        char *cmd;
        while (line != NULL) {
            cmd = strsep(&line, PARALLEL_DELIMETER);
            if (strcmp(cmd, "") == 0) continue;
            cmds[cmd_count] = cmd;
            cmd_count++;
        }

        int cmdi;
        char *temp, *filename = NULL;
        int filenamec = 0;
        char *filenamev[MAX_ARGC];
        for (cmdi = 0; cmdi < cmd_count; cmdi++) {
            // Get redirection information
            // TODO: what is the best practice to deal with changeable char*
            temp = cmds[cmdi];
            cmd = strsep(&temp, REDIRECT_DELIMETER);

            // Redirection file found
            if (temp != NULL) {
                filename = strsep(&temp, REDIRECT_DELIMETER);

                // Check filename
                // TODO: encapsulate; how to do this?
                // No multiple redirections
                if (temp != NULL) {
                    __error();
                    break;
                }

                parse_command(filename, &filenamec, filenamev);
                if (filenamec != 1) {
                    __error();
                    break;
                }
                filename = *filenamev;
            }

            parse_command(cmds[cmdi], &argc, argv);
            execute_command(filename, argc, argv);
        }

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
    for (int i = 0; i < MAX_PATH_COUNT; i++) 
        paths[i] = malloc(sizeof(char) * MAX_PATH_LENGTH);
    strcpy(paths[0], "/bin");
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
            // perror("Fail to open the file!\n");
            __error();
            exit(1);
        }
    }

    // Invalid arguments
    else {
        // printf("wish shell can only take one or two arguments!\n");
        __error();
        exit(1);
    }

    // Execute command input by the stream
    rc = execute_commands(istream);

    // Free space
    for (int i = 0; i < MAX_PATH_COUNT; i++) free(paths[i]);
    return rc;
}