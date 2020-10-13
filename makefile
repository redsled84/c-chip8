OBJS = main.c chip8.c

CC = gcc

COMPILER_FLAGS = -w

LINKER_FLAGS = -lSDL2

OBJ_NAME = main

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
