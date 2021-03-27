#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"

/*
 * Replaces occurrences of certain substrings in the source string,
 *  returns the interpolated string in the destination.
 *
 *  Replaces:
 *  $$ -> pid
 */
void interpolate(char* source, char* destination)
{
  // get pid string
  char pid[10];
  memset(pid, '\0', sizeof(pid));
  sprintf(pid, "%d", getpid());

  // setup pointers for source and destination strings
  char* pDes = destination;
  char* pSrc = source;
  while (*pSrc != '\0')
  {
    // check if pointing at $$
    if (pSrc == strstr(pSrc, "$$")) {
      dprint(("*** replacing $$ with %s, size %lu\n", pid, strlen(pid)));

      strcat(pDes, pid);

      // move pointers ahead
      pSrc += strlen("$$");
      pDes += strlen(pid);
    }
    else
    {
      // push characters to interpolated string
      *pDes++ = *pSrc++;
    }
  }

  *pDes = '\0';

  dprint(("*** %s\n", destination));
}


/*
 * Parses a string into tokens by whitespace,
 *  returns the tokens in tokens.
 */
void parse(char *buffer, char **tokens)
{
  dprint(("*** parsing %s\n", buffer));

  // continue to end of the buffer
  char* ptr = buffer;
  while (*ptr != '\0')
  {
    // replace whitespace with null
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')
    {
        *ptr++ = '\0';
    }

    // save positional arg
    if (*ptr != '\0')
    {
      dprint(("*** saving arg: %s\n", ptr));
      *tokens++ = ptr;
    }

    // advance passed characters
    while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
    {
        dprint(("%c ", *ptr));
        ptr++;
    }

    dprint(("\n"));
  }

  // mark last entry null
  *tokens = NULL;
}


/*
 * Replaces comments with blank space.
 */
void nullifyComments(char *buffer)
{
  char* ptr = buffer;
  while (*ptr != '\0')
  {
    if (*ptr == '#')
    {
      while (*ptr != '\n' && *ptr != '\0')
        *ptr++ = ' ';
    }
    ptr++;
  }
}