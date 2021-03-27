#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "execute.h"


/*
 * Terminate current processes and exit shell.
 */
void exitCmd(int argc, int *flag, pid_t *processes)
{
    if (argc != 1)
    {
        *flag = 1;
        perror("exit: illegal option"); fflush(stdout);
    }
    else
    {
        // quit bg processes
        reap(processes, flag, 1);

        exit(0);
    }
}


/*
 * Change current directory.
 */
void cdCmd(int argc, char** argv, int* flag)
{
    if (argc == 1)
    {
        dprint(("*** No directory provided, changing pwd to %s\n", getenv("HOME")));

        // changing to HOME dir
        if (chdir(getenv("HOME")) != 0) {
            *flag = errno;
            perror("cd"); fflush(stdout);
        }
        else
            *flag = 0;

        // debug print cwd
        char directory[1024];
        getcwd(directory, sizeof(directory));
        dprint(("*** Current working directory changed to: %s\n", directory));
    }
    else if (argc == 2)
    {
        dprint(("*** Changing working directory to %s\n", argv[1]));

        // changing to provided dir
        if (chdir(argv[1]) != 0) {
            *flag = errno;
            perror("cd"); fflush(stdout);
        }
        else
            *flag = 0;

        char directory[1024];
        getcwd(directory, sizeof(directory));
        dprint(("*** Current working directory changed to: %s\n", directory));
    }
    else
    {
        *flag = 1;
        perror("cd: illegal option"); fflush(stdout);
    }
}


/*
 * Prints the status flag.
 */
void statusCmd(int argc, int* flag)
{
    printf("%d\n", *flag); fflush(stdout);
}