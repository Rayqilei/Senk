CC=gcc
CFLAGS=-std=c99 -mwindows -o SenkTrojan.exe -w -s
SRC=main.c

all:
	$(CC) $(CFLAGS) $(SRC)