#ifndef SHELL_OPERATORS_H
#define SHELL_OPERATORS_H

int handleOperators(int argc, char **argv, int *statusFlag, int *fds, int *bgFlag);
void resetFDs(int* fds);
int changeSTDIN(char* newInput, int* statusFlag, int *fds);
int changeSTDOUT(char* newOutput, int* statusFlag, int *fds);

#endif //SHELL_OPERATORS_H
