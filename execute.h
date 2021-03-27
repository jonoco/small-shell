#ifndef SHELL_EXECUTE_H
#define SHELL_EXECUTE_H

void execute(char **argv, int *statusFlag, int bgFlag, pid_t *bgProcesses, int *fds);
void waitChild(pid_t pid, int* statusFlag);
void reap(pid_t *bgProcesses, int *statusFlag, int quitFlag);
void catchSIGINT (int signo);

#endif //SHELL_EXECUTE_H
