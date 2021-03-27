#ifndef SHELL_PARSE_H
#define SHELL_PARSE_H

void nullifyComments(char *buffer);
void parse(char *buffer, char **argv);
void interpolate(char* source, char* destination);

#endif