#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"

/*
 * Reset file descriptors to original and close previous open ones
 */
void resetFDs(int* fds)
{
    for (int i = 0; i < 3; ++i) {
        if (fds[i] != i) {
            if (dup2(fds[i], i) == -1) {
                perror("fds"); fflush(stdout);
            }
            close(fds[i]);
            fds[i] = i;
        }
    }
}


/*
 * Change the file descriptor for stdin
 */
int changeSTDIN(char* newInput, int* statusFlag, int *fds)
{
    // save previous fd for stdin
    int stdinFD = dup(0);

    // open new fd
    int sourceFD = open(newInput, O_RDONLY);
    if (dup2(sourceFD, 0) == -1) {
        perror("<"); fflush(stdout);
        *statusFlag = 1;
        return -1;
    }
    else {
        dprint(("*** opened %s at fd %d\n", newInput, sourceFD));

        fds[0] = stdinFD;
        return stdinFD;
    }
}


/*
 * Change the file descriptor for stdout
 */
int changeSTDOUT(char* newOutput, int* statusFlag, int *fds)
{
    // save previous fd for stdout
    int stdoutFD = dup(1);

    // open new fd
    int targetFD = open(newOutput, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dup2(targetFD, 1) == -1) {
        perror(">"); fflush(stdout);
        *statusFlag = 1;
        return -1;
    }
    else {
        dprint(("*** opened %s at fd %d\n", newOutput, targetFD));

        fds[1] = stdoutFD;
        return stdoutFD;
    }
}


/*
 * Check for operators: < > &
 * Performs necessary operations if an operator is found
 */
int handleOperators(int argc, char **argv, int *statusFlag, int *fds, int *bgFlag)
{
    int ret = 0;

    for (int i = argc-1; i >= 0; i--)
    {   // check for input operator <
        if (strcmp(argv[i], "<") == 0 && i != argc-1) {
            dprint(("*** input redirecting to %s on arg %d\n", argv[i+1], i));

            if (changeSTDIN(argv[i+1], statusFlag, fds) > 0) {
                // shift arguments, eliminating operator and input file
                argv[i] = argv[i+2];
                argv[i+1] = argv[i+3];
                argv[i+2] = NULL;
                argv[i+3] = NULL;
            }
            else {
                ret = 1;
            }
        } // check for output operator >
        else if (strcmp(argv[i], ">") == 0 && i != argc-1) {
            dprint(("*** output redirecting to %s on arg %d\n", argv[i+1], i));

            if (changeSTDOUT(argv[i+1], statusFlag, fds) > 0) {
                // shift arguments, eliminating operator and output file
                argv[i] = argv[i+2];
                argv[i+1] = argv[i+3];
                argv[i+2] = NULL;
                argv[i+3] = NULL;
            }
            else {
                ret = 1;
            }
        } // check for background operator &
        else if (strcmp(argv[i], "&") == 0 && i == argc-1) {
            dprint(("*** background operator & on arg %d\n", i));

            *bgFlag = 1;

            // eliminate argument
            argv[i] = NULL;
        }
    }

    return ret;
}