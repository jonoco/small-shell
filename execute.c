#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "debug.h"
#include "operators.h"


/*
 * Print exit status and set flag
 */
void checkExitStatus(pid_t pid, int childExitStatus, int *statusFlag, int bgPrint, int fgPrint)
{
    if (WIFEXITED(childExitStatus)) // check for normal exit
    {
        *statusFlag = WEXITSTATUS(childExitStatus);

        dprint(("*** process exited normally - exit status was %d\n", *statusFlag));

        // print message for exiting bg processes
        if (bgPrint) {
            printf("background pid %d is done: exited with status %d\n", pid, *statusFlag); fflush(stdout);
        }
    }
    else if (WIFSIGNALED(childExitStatus)) // check for signal exit
    {
        *statusFlag = WTERMSIG(childExitStatus);

        dprint(("*** process exited by signal - signal was %d\n", *statusFlag));

        // print messages for exiting bg or fg processes
        if (bgPrint) {
            printf("background pid %d is done: terminated with signal %d\n", pid, *statusFlag); fflush(stdout);
        }
        else if (fgPrint) {
          printf("terminated with signal %d\n", *statusFlag); fflush(stdout);
        }
    }
}


/*
 * Wait for a child process without hanging
 * Returns child pid if finished, otherwise 0
 */
int waitChild(pid_t pid, int* statusFlag)
{
    int childExitStatus = -5;
    int childPid = waitpid(pid, &childExitStatus, WNOHANG);
    if (childPid != 0) {
        checkExitStatus(childPid, childExitStatus, statusFlag, 1, 0);
        return childPid;
    }
    else {
        return 0;
    }
}


/*
 * Wait for a child process with hanging
 */
void hangWaitChild(pid_t pid, int* statusFlag)
{
    dprint(("*** waiting for child %d\n", pid));

    int childExitStatus = -5;

    // restart wait if interrupted
    if (waitpid(pid, &childExitStatus, 0) == -1) {
        dprint(("*** caught error waiting for %d -- restarting wait\n", pid));

        hangWaitChild(pid, statusFlag);
    }
    else {
        dprint(("*** finished waiting for child %d\n", pid));

        checkExitStatus(pid, childExitStatus, statusFlag, 0, 1);
    }
}


/*
 * Reap bg children
 */
void reap(pid_t *bgProcesses, int *statusFlag, int quitFlag)
{
    // look for active bg processes
    for (int i = 0; i < 100; ++i) {
        if (bgProcesses[i] != 0) {
            dprint(("*** checking bg pid: %d\n", bgProcesses[i]));

            // check if quitting or waiting for processes
            if (quitFlag) {
                dprint(("*** quitting pid %d\n", bgProcesses[i]));

                kill(bgProcesses[i], SIGQUIT);
            } else {
                // check if child finished
                if (waitChild(bgProcesses[i], statusFlag)) {
                    bgProcesses[i] = 0;
                }
            }
        }
    }
}


/*
 * Execute new fg process
 */
void execFGProcess(char** argv, int* statusFlag)
{
    dprint(("*** executing fg process\n"));

    // setup sig handler for INT and TSTP
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    struct sigaction SIGINT_action = {0};
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_handler = SIG_DFL;
    sigaction(SIGINT, &SIGINT_action, NULL);

    if (execvp(*argv, argv) < 0) {
        perror("execvp"); fflush(stdout);
        *statusFlag = 1;
        exit(1);
    }
}


/*
 * Execute new bg process
 */
void execBGProcess(char** argv, int* statusFlag, int* fds)
{
    dprint(("*** executing bg process\n"));

    // setup sig handlers for INT and TSTP
    struct sigaction ignore_action = {0};
    sigfillset(&ignore_action.sa_mask);
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);
    sigaction(SIGTSTP, &ignore_action, NULL);

    // check if stdin or stdout changed, otherwise redirect to null
    if (fds[0] == 0) {
        changeSTDIN("/dev/null", statusFlag, fds);
    }
    if (fds[1] == 1) {
        changeSTDOUT("/dev/null", statusFlag, fds);
    }

    if (execvp(*argv, argv) < 0) {
        perror("execvp"); fflush(stdout);
        *statusFlag = 1;
        exit(1);
    }
}


/*
 * Execute command and set status flag
 */
void execute(char **argv, int *statusFlag, int bgFlag, pid_t *bgProcesses, int *fds)
{
    dprint(("*** executing %s\n", *argv));

    pid_t pid = -5;
    if ((pid = fork()) < 0) {
        perror("fork"); fflush(stdout);
        exit(1);
    }
    else if (pid == 0) {
        // check if child is a background or foreground process
        if (bgFlag) {
            execBGProcess(argv, statusFlag, fds);
        }
        else {
            execFGProcess(argv, statusFlag);
        }
    }
    else {
        // check if process is kept in background or foreground
        if (bgFlag) {
            printf("background pid is %d\n", pid);

            // add process to background array
            pid_t* ptr = bgProcesses;
            while(*ptr != 0) { ptr++; }
            *ptr = pid;
        }
        else {
            hangWaitChild(pid, statusFlag);
        }
    }
}