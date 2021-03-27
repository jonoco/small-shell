CC=gcc
CFLAGS=-I -Wall -std=c99

smallsh: commands.c execute.c main.c operators.c parse.c
		$(CC) $(CFLAGS) -o smallsh commands.c execute.c main.c operators.c parse.c