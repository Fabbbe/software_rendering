CC=gcc
CFLAGS=$(shell pkg-config --cflags --libs sdl2) -lm -O2 #-Iinclude/

default: src/main.c
	$(CC) $(CFLAGS) -o softRend src/main.c
