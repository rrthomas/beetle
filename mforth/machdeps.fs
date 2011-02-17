\ Machine-dependent words (Beetle)
\ Reuben Thomas   2/7/96-2/4/97


\ System variables

: LIMIT   4 ;


\ Compiler #1

: (S")   R>  DUP C@  TUCK 1+ OVER + ALIGNED  >R  1+ SWAP ;


\ Branches

HEX
: FITS   ( x addr -- flag )   DUP ALIGNED >-<  8 * 1-  1 SWAP LSHIFT
   SWAP DUP 0< IF  INVERT  THEN  U> ;
: FIT,   ( x -- )   HERE DUP ALIGNED >-<  0 ?DO  DUP C,  8 RSHIFT  LOOP
   DROP ;
   
: NOPALIGN   0 FIT, ;

: AHEAD   HERE  42 C,  NOPALIGN  0 , ; IMMEDIATE
: IF   HERE  44 C,  NOPALIGN  0 , ; IMMEDIATE

: OFFSET   ( from to -- offset )   >-<  CELL/  00FFFFFF AND ;
: !BRANCH   ( at from to opcode -- )   HERE >R  >R  ROT DP !  R> C,  OFFSET
   FIT,  R> DP ! ;

: BRANCH   ( at from to -- )   43 !BRANCH ;
: CALL   ( at from to -- )   49 !BRANCH ;

: JOIN   ( from to -- )   SWAP  1+ ALIGNED  ! ;

: ADR,   ( to opcode -- )   OVER HERE 1+ ALIGNED - CELL/  DUP HERE 1+ FITS
   IF  SWAP 1+ C, FIT,  DROP  ELSE DROP C,  NOPALIGN  ,  THEN ;
: CALL,   ( to -- )   48 ADR, ;
: COMPILE,   DUP >INFO 2 + C@  ?DUP IF  0 DO  DUP C@ C,  1+  LOOP  DROP
   ELSE CALL,  THEN ;

: LINK, ;
: UNLINK,   4A C, ;
: LEAVE,   50 C,  42 C, ;
DECIMAL


\ Compiler #2

: (POSTPONE)   R>  DUP 4 + >R  @ COMPILE, ;

HEX
: DO,   4B C, ;
: LOOP,   4C ADR, ;
: +LOOP,   4E ADR, ;
: UNLOOP, ;

ALSO ASSEMBLER
: EXECUTE   STATE @ IF  46 C,  NOPALIGN  ELSE [ 46 C,  NOPALIGN ]
   THEN ; IMMEDIATE
: @EXECUTE   STATE @ IF  47 C,  NOPALIGN  ELSE [ 47 C,  NOPALIGN ]
   THEN ; IMMEDIATE
PREVIOUS
DECIMAL


\ Data structures

HEX
: LITERAL   DUP HERE 1+ FITS IF  53 C, FIT,  ELSE 52 C,  NOPALIGN  ,  THEN ;
IMMEDIATE

: CREATE,   LINK,  56 C,  UNLINK,  NOPALIGN ;
DECIMAL


\ Terminal input/output

: BL   0 LIB ;
: CR   1 LIB ;
: EMIT   2 LIB ;
: DEL   8 EMIT  BL EMIT  8 EMIT ;
: KEY   3 LIB ;

: DEL?   DUP 127 =  SWAP 8 =  OR ;
: CR?   DUP 13 =  SWAP 10 =  OR ;
: EOL?   10 = ;

77 CONSTANT WIDTH   \ width of display