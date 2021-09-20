#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>

#define MAX_ARGC 16
#define MAX_PATH_COUNT 32
#define MAX_PATH_LENGTH 32
#define MAX_EXECUTABLE_LENGTH 128
#define MAX_CMD_COUNT 32

const char *PROMPT = "wish> ";

const char *DELIMETER = " \n";
const char *REDIRECT_DELIMETER = ">";
const char *PARALLEL_DELIMETER = "&";

const char *EXIT_COMMAND = "exit";
const char *CD_COMMAND = "cd";
const char *PATH_COMMAND = "path";

void __error();
void __exit();
void __cd(const char *arg);
void __path(int argc, char *argv[]);
void parse_command(char *command, int *argc, char **argv);
int execute_command(char *filename, int argc, char *argv[]);
int execute_commands(FILE *file);

