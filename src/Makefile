# Makefile for C Beetle
# Reuben Thomas   11/94-17/2/11


# The next few lines are user modifiable

# The names of your C compiler and linker; here we assume it's GCC
CC = gcc
LD = gcc -o $@

# If your machine is big-endian, you should uncomment the next line
#ENDISM = -DBEETLE_BIG_ENDIAN

# Flags for the C compiler; the uncommented version of CCflags is recommended
# as the commented one is meant for development
# If you change CCflags, make sure it still ends in $(ENDISM)
# add -DB_DEBUG for debugging output
CFLAGS = -O3 $(ENDISM) -DB_DEBUG
#CFLAGS = -pedantic-errors -Wall $(ENDISM)


# The rest of the file is not user-modifiable

BeetleObjs = storage.o tests.o step.o run.o lib.o debug.o loadobj.o saveobj.o noecho.o


# Targets

all:		beetle uiface

clean:
		rm $(BeetleObjs) beetle uiface

check:		runt

dist:
		cd .. && zip -r beetle.zip src/

beetle:		beetle.o $(BeetleObjs)
	$(LD)	$@.o $(BeetleObjs)

uiface:		uiface.o $(BeetleObjs)
	$(LD)	$@.o $(BeetleObjs)

runt:		runt.o $(BeetleObjs)
	$(LD)	$@.o $(BeetleObjs)


# Other dependencies

step.o:		beetle.h bportab.h opcodes.h execute.c excepts.c
run.o:		beetle.h opcodes.h execute.c excepts.c
storage.o:	beetle.h bportab.h
debug.o:	beetle.h bportab.h opcodes.h debug.h
