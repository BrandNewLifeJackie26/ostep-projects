#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

const char* PROMPT = "wish> ";

void __error();
void __exit();
void __cd(const char *arg);
void __path(void *argv[]);
int execute_command(void *args);
int execute_commands(FILE *file);

