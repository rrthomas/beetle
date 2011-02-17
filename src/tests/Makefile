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
CFLAGS = -O3 $(ENDISM) -DB_DEBUG -I..
#CFLAGS = -pedantic-errors -Wall $(ENDISM)


# The rest of the file is not user-modifiable

BeetleObjs = ../storage.o ../tests.o ../step.o ../run.o ../lib.o \
	     ../debug.o ../loadobj.o ../saveobj.o ../noecho.o
TestHeaders = beetle.h opcodes.h debug.h btests.h
BTests = aritmtit brancht comparet createt doloopt initialt libt literalt \
	logict memoryt stackt stept runt loadobjt saveobjt exceptst linkt
BTestsO = aritmtit.o brancht.o comparet.o createt.o doloopt.o initialt.o \
	libt.o literalt.o logict.o memoryt.o stackt.o stept.o runt.o \
	loadobjt.o saveobjt.o exceptst.o linkt.o


# Targets

all:		btests

clean:
		rm $(BTests) $(BTestsO)


btests:		$(BTests)

$(BTests):	$(BTestsO) $(BeetleObjs)
	$(LD)	$@.o $(BeetleObjs)
