#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

void exitCmd(int argc, int *flag, pid_t *processes);
void cdCmd(int argc, char** argv, int* flag);
void statusCmd(int argc, int* flag);

#endif //SHELL_COMMANDS_H
