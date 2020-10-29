OBJS = chip8.c main.c

CC = gcc

COMPILER_FLAGS = -w

LINKER_FLAGS = -lSDL2 -lm

OBJ_NAME = main

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
