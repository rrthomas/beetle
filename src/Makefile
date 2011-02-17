# Makefile for C Beetle
# Reuben Thomas   11/94-17/2/11


include defaults.mak

CFLAGS += -I..


BeetleObjs = storage.o tests.o step.o run.o lib.o debug.o loadobj.o saveobj.o noecho.o


# Targets

all: beetle

clean:
	rm -f $(BeetleObjs) beetle beetle.o runt.o

check: runt
	./runt

dist:
	cd .. && zip -r beetle.zip src/

beetle: beetle.o $(BeetleObjs)
	$(CC) -o $@ $@.o $(BeetleObjs)

runt: runt.o $(BeetleObjs)
	$(CC) -o $@ $@.o $(BeetleObjs)


# Other dependencies

step.o: beetle.h opcodes.h
run.o: beetle.h opcodes.h
storage.o: beetle.h
debug.o: beetle.h opcodes.h debug.h
