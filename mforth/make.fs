\ Compile mForth under aForth
\ Reuben Thomas   15/4/96-9/4/99


CHARS TUCK  PAD 19 +  SWAP CMOVE
CR .( Metacompiling mForth for Beetle: )

INCLUDE assembler/fs


\ Meta-compiler utilities

VOCABULARY META  ALSO META DEFINITIONS
DECIMAL

\ Relocate new dictionary
: RELOCATE   ( adr -- )
   CURRENT-VOLUME @ @  #THREADS CELLS  OVER + SWAP DO
      I @  ?DUP IF  <M0 I !  THEN
   CELL +LOOP
   BEGIN
      >LINK
      DUP @ ?DUP WHILE
      OVER CELL+   \ hack to go from link field to thread field
      DUP @  ?DUP IF  <M0 SWAP !  ELSE DROP  THEN
      DUP <M0 ROT !
   REPEAT
   DROP ;


INCLUDE meta/fs


VOLUME NEW   \ define a new hash table
NEW VOCABULARY NEW-FORTH   \ define the new root vocabulary
NEW   \ make the new dictionary the current volume

SIZE DICTIONARY CROSS  \ define a new dictionary
' CROSS >BODY @  #THREADS 1+ CELLS  -  TO M0
   \ make M0 point to the start of it minus the threads table and initial
   \ branch

ALSO CROSS NEW-FORTH DEFINITIONS FOREIGN

>COMPILERS<
M0 CELL+  <M0   \ address of start of threads hash table
'THROW-CONTENTS <M0   \ address of contents of 'THROW

ALSO ASSEMBLER
INCLUDE primitives/fs
PREVIOUS

INCLUDE highlevel/fs

INCLUDE initialize/fs
INCLUDE patches/fs
' NEW-FORTH >BODY @ @  PREVIOUS  DUP RELOCATE   \ relocate the new dictionary
>COMPILERS<

ALIGN HERE M0 -   \ ( length ) of binary image
ROOT HERE OVER ALLOT   \ make space for binary image ( length start )
TUCK   \ ( start length start )
M0  #THREADS 1+ CELLS   \ ( s l s M0 (#THREADS+1)CELLS )
TUCK + -ROT +   \ ( s l M0+(#T+1)CELLS H+(#T+1)CELLS )
2 PICK MOVE   \ copy dictionary ( s l )
OVER CELL+ CURRENT-VOLUME @ @  SWAP   \ ( s l 'THREADS s+CELL )
#THREADS CELLS MOVE   \ copy threads ( s l )

INCLUDE branch/fs

S" mImg"  SAVE   \ write object module

KERNEL PREVIOUS DEFINITIONS   \ restore original order
