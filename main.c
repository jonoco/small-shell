#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "parse.h"
#include "execute.h"
#include "debug.h"
#include "commands.h"
#include "operators.h"


int getInput(char** argv, int* argc);
void catchSIGTSTP(int signo);
void checkFGMode();
void setupSigHandlers();


volatile sig_atomic_t fgModeFlag = 0;         // foreground mode flag
volatile sig_atomic_t setfgModeFlag = 0;      // fgModeFlag observer


int main()
{
    // check debug flag
    dprint(("*** debug printing is on\n"));

    int argc;                   // argument count
    char* argv[550];            // arguments array
    int statusFlag = 0;         // shell status flag
    int fds[3] = { 0, 1, 2 };   // open file descriptors for in, out, err
    pid_t bgProcesses[100];     // background processes
    memset(bgProcesses, 0, sizeof(bgProcesses));

    setupSigHandlers();

    while(1) {
        // get parsed input from user
        if (getInput(argv, &argc) != 0) {
            reap(bgProcesses, &statusFlag, 0);
            continue;
        }

        // parse arguments for special operators, then handle them
        int bgFlag = 0;
        if (handleOperators(argc, argv, &statusFlag, fds, &bgFlag) != 0) {
            continue;
        }

        // check built in commands
        if (strcmp(argv[0], "exit") == 0) {
            exitCmd(argc, &statusFlag, bgProcesses);
        }
        else if (strcmp(argv[0], "cd") == 0) {
            cdCmd(argc, argv, &statusFlag);
        }
        else if (strcmp(argv[0], "status") == 0) {
            statusCmd(argc, &statusFlag);
        }
        else {
            // check for foreground mode
            int bg = fgModeFlag ? 0 : bgFlag;

            execute(argv, &statusFlag, bg, bgProcesses, fds);
        }

        // reset file descriptors
        resetFDs(fds);

        // check on children
        reap(bgProcesses, &statusFlag, 0);

        // check if Ctrl-Z called during fg process
        checkFGMode();
    }

    exit(0);
}


/*
    Get and parse user input
    Returns 0 for valid input, otherwise 1
 */
int getInput(char** argv, int* argc)
{
    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t input;

    // start prompt
    while(1)
    {
        printf(": "); fflush(stdout);
        input = getline(&buffer,&bufsize,stdin);
        if (input == -1) {
            // clear err and check for TSTP switch
            clearerr(stdin);
            checkFGMode();
        }
        else
            break;
    }

    // remove trailing \n
    buffer[input-1] = '\0';

    dprint(("*** got input: %s\n", buffer));

    // ignore comments
    if (buffer[0] == '#') {
        free(buffer);
        return 1;
    }

    // interpolate special characters
    char* interBuffer = calloc(2048, sizeof(char));
    interpolate(buffer, interBuffer);

    // tokenize input
    parse(interBuffer, argv);

    // count parsed arguments
    *argc = 0;
    char **ptr = argv;
    while (*ptr != NULL) {
        dprint(("*** parsed %d: %s\n", *argc, *ptr));
        (*argc)++;
        ptr++;
    }

    free(buffer);
    free(interBuffer);

    // ignore empty lines
    if (*argc == 0)
        return 1;

    return 0;
}


/*
 * Check if the fgModeFlag flipped
 */
void checkFGMode()
{
    dprint(("*** Checking fg flag\n"));

    if (setfgModeFlag != fgModeFlag) {
        fgModeFlag = setfgModeFlag;
        if (fgModeFlag)
            printf("\nEntering foreground-only mode (& is now ignored)\n");
        else
            printf("\nExiting foreground-only mode\n");
    }
}


/*
 * Flip foreground mode flag
 */
void catchSIGTSTP(int signo)
{
    setfgModeFlag = fgModeFlag == 0 ? 1 : 0;
}


/*
 * Setup signal handlers for shell process
 */
void setupSigHandlers()
{
    dprint(("*** Setting up signal handlers\n"));

    // Handle INT, ignore for shell process
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Handle TSTP, flip foreground process switch
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}


