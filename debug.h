#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define dprint(x) do { if (getenv("DEBUG")) { printf x; fflush(stdout); }} while(0)

#endif