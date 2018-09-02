# Project: libasm

CPP  = g++
CC   = gcc
OBJ  = obj/asm.o $(RES)
LINKOBJ  = obj/asm.o $(RES)
BIN  = libasm.a
CFLAGS = -O2 -Wall
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before libasm.a all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(LINKOBJ)
	ar r $(BIN) $(LINKOBJ)
	ranlib $(BIN)

obj/asm.o: src/asm.c
	$(CC) $(CFLAGS) -c src/asm.c -o obj/asm.o
