CC=clang++
CFLAGS+=-g -Wall -Wextra -Wno-self-assign -Wno-unused-parameter
LDFLAGS+=-lGL -lGLEW -lglfw -lIL -lILU -lILUT
INCLUDEFLAGS=-I/usr/include/eigen3
CFILES=*.cpp
OFILES=*.o
NAME=sim

all:
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) $(LDFLAGS) -o $(NAME) $(CFILES)
