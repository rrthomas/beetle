\ Save an object file
: SAVE   ( a-addr u1 c-addr u2 -- )
   2SWAP 2OVER                   \ save filename
   W/O CREATE-FILE DROP          \ open file
   >R                            \ save file-id
   S" BEETLE" R@ WRITE-FILE DROP \ write header
   0 SCRATCH TUCK C!  1  2DUP  R@ WRITE-FILE DROP
   R@ WRITE-FILE DROP
   DUP CELL/ SCRATCH TUCK !  CELL R@ WRITE-FILE DROP
   R@ WRITE-FILE DROP            \ write data
   R> CLOSE-FILE DROP ;          \ close file

\ Create bForth assembler primitives
: PRIMITIVES   ( +n -- )
   0 DO
      [CHAR] B PAD CHAR+ C!      \ store "B" at start of name
      BL WORD  COUNT TUCK        \ get name
      PAD 2 CHARS + SWAP CMOVE   \ append it to the "B"
      CHAR+ DUP NEGATE >IN +!    \ move >IN back over the name
      PAD  TUCK C!               \ save PAD; store name's length
      CODE                       \ make an inline code word
      [ ALSO ASSEMBLER ]
      1 INLINE                   \ with one byte of code
      FIND DROP  EXECUTE         \ append the opcode      
      BEXIT                      \ append EXIT
      END-CODE                   \ finish the definition
      [ PREVIOUS ]
   LOOP ;


\ Compiler redefinition and additions

: V'   ' CELL + ;

ALSO ASSEMBLER
: ADR,   ( to opcode -- )   OVER HERE 1+ ALIGNED - CELL/  DUP HERE 1+ FITS
   IF  SWAP 1+ C, FIT,  DROP  ELSE DROP C,  0 FIT,  <M0 ,  THEN ;

HEX
: EXECUTE   STATE @ IF  46 C,  ALIGN  ELSE  EXECUTE  THEN ; IMMEDIATE
: @EXECUTE   STATE @ IF  47 C,  ALIGN  ELSE  @EXECUTE  THEN ; IMMEDIATE

R: <M0   M0 - 10 + ;
R: AHEAD   HERE  42 C,  0 FIT,  0 , ;
R: IF   HERE  44 C,  0 FIT,  0 , ;
R: LITERAL   B(LITERAL) ;
R: NOPALIGN   0 FIT, ;
R: OFFSET   ( from to -- offset )   >-<  CELL/  00FFFFFF AND ;
R: !BRANCH   ( at from to opcode -- )   -ROT  OFFSET  8 LSHIFT  OR  SWAP ! ;
R: BRANCH   ( at from to -- )   43 !BRANCH ;
R: CALL   ( at from to -- )   49 !BRANCH ;
R: JOIN   ( from to -- )   <M0  SWAP 1+ ALIGNED  ! ;
R: CALL,   ( to -- )   48 ADR, ;
R: COMPILE,   DUP >INFO 2 + C@  ?DUP IF  0 DO  DUP C@ C,  1+  LOOP  DROP
   ELSE CALL,  THEN ;
R: LINK, ; IMMEDIATE
R: UNLINK,   4A C, ;
R: DO,   4B C, ;
R: LOOP,   4C ADR, ;
R: +LOOP,   4E ADR, ;
R: LEAVE,   50 C, 42 C, ;
R: I   0E C, ;
R: ?DO   'NODE @  0 'NODE !  04 C, 04 C,  DO,  11 C,  POSTPONE IF
   POSTPONE LEAVE POSTPONE THEN POSTPONE BEGIN ;
R: >NODE   'NODE @  BEGIN  ?DUP WHILE  DUP @  HERE <M0 ROT !  REPEAT ;
R: LOOP   LOOP,  >NODE  'NODE ! ;
R: +LOOP   +LOOP,  >NODE  'NODE ! ;
R: CREATE,   LINK,  56 C,  UNLINK,  0 FIT, ;
R: (POSTPONE)   R>  03FFFFFF AND  DUP 4 + >R  @ >NAME FIND  0= IF  ERROR" ?"
   THEN  COMPILE, ;
R: POSTPONE   BL WORD FIND  ?DUP 0= IF  ERROR" ?"  THEN  0> IF  COMPILE,
   ELSE POSTPONE (POSTPONE) ALIGN  <M0 ,  THEN ;
DECIMAL
26 REDEFINER >COMPILERS<

PREVIOUS


\ Constants and patch phrases

24 1024 * CONSTANT SIZE
: 'THROW-CONTENTS   M0 16 - ;