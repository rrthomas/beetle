# Makefile for C Beetle
# Reuben Thomas   11/94-17/2/11


include ../defaults.mak

CFLAGS += -I.. -I../..


BeetleObjs = ../storage.o ../tests.o ../step.o ../run.o ../lib.o \
	     ../debug.o ../loadobj.o ../saveobj.o ../noecho.o
BTests = aritmtit brancht comparet createt doloopt initialt libt literalt \
	logict memoryt stackt stept runt loadobjt saveobjt exceptst linkt
BTestsO = aritmtit.o brancht.o comparet.o createt.o doloopt.o initialt.o \
	libt.o literalt.o logict.o memoryt.o stackt.o stept.o runt.o \
	loadobjt.o saveobjt.o exceptst.o linkt.o


# Targets

check: $(BTests)
	./aritmtit
	./brancht
	./comparet
	./createt
	./doloopt
	./initialt
	./libt
	./literalt
	./logict
	./memoryt
	./stackt
	./stept
	./runt
	./loadobjt
	./saveobjt
	./exceptst
	./linkt

clean:
	rm $(BTests) $(BTestsO)

$(BTests): $(BTestsO) $(BeetleObjs)
	$(CC) -o $@ $@.o $(BeetleObjs)
