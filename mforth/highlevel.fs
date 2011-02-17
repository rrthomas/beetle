\ mForth kernel high level words
\ Reuben Thomas   1/91-2/4/97

CR .( High level words )


\ System variables

CREATE 'THROW ,   \ value is put on stack by make/fs
CREATE 'THREADS ,   \ value is put on stack by make/fs
VARIABLE S0
VARIABLE R0


\ Stack manipulation

: 2DUP   OVER OVER ;
: 2DROP   DROP DROP ;
: 2SWAP   3 ROLL  3 ROLL ;
: 2OVER   3 PICK  3 PICK ;

: 2>R   R> -ROT  SWAP >R >R  >R ;
: 2R>   R>  R> R> SWAP  ROT >R ;

: DEPTH   SP@ S0 @ >-< CELL/ ;


\ Interpreter #1

: (ABORT")   ROT IF  -2 THROW  ELSE 2DROP  THEN ;
: (ERROR")   -512 THROW ;


\ Terminal input/output #1

: COUNT   DUP 1+ SWAP C@ ;


\ Compiler #1

VARIABLE DP
: ALLOT   DP +! ;
: HERE   DP @ ;

: ALIGNED   CELL+ 1-  -CELL AND ;
: ALIGN   HERE ALIGNED  DP ! ;
: ,   HERE  CELL ALLOT  ! ;
: C,   HERE  1 ALLOT  C! ;

VARIABLE STATE
: [   0 STATE ! ; IMMEDIATE
: ]   1 STATE ! ;

VARIABLE CONTEXT
: LAST   CONTEXT @ ;

: >LINK   3 CELLS - ;
: >THREAD   2 CELLS - ;
: >INFO   CELL- ;
: >NAME   DUP >INFO 3 + C@  31 AND 1+ ALIGNED  SWAP >LINK  >-< ;
: >BODY   2 CELLS + ;

HEX
: IMMEDIATE   LAST >INFO  DUP @  80000000 OR  SWAP ! ;
: SMUDGE!   ( f adr -- )   >INFO  TUCK @  40000000 DUP INVERT  ROT AND
   -ROT AND  OR  SWAP ! ;
DECIMAL
: SMUDGE   ( f -- )   LAST  SMUDGE! ;


INCLUDE Machine:machdeps/fs   \ include machine-dependent words


\ Interpreter #2

: PAD   R0 @ 256 + ;
: TOKEN   R0 @ 512 + ;
: SCRATCH   R0 @ 768 + ;
: S"B   R0 @ 1024 + ;

: ABORT   -1 THROW ;
: QUIT   -56 THROW ;


\ Control structures #1

: BEGIN   NOPALIGN HERE ; IMMEDIATE
: AGAIN   POSTPONE AHEAD  SWAP JOIN ; IMMEDIATE
: UNTIL   POSTPONE IF  SWAP JOIN ; IMMEDIATE
: THEN   NOPALIGN HERE JOIN ; IMMEDIATE

: WHILE   POSTPONE IF  SWAP ; IMMEDIATE
: REPEAT   POSTPONE AGAIN  POSTPONE THEN ; IMMEDIATE
: ELSE   POSTPONE AHEAD  SWAP  POSTPONE THEN ; IMMEDIATE

VARIABLE 'NODE
: >NODE   'NODE @  BEGIN  ?DUP WHILE  DUP @  HERE ROT !  REPEAT ;
: I   POSTPONE R@ ; IMMEDIATE
: LEAVE   LEAVE, NOPALIGN  HERE 'NODE  DUP @ ,  ! ; IMMEDIATE
: DO   'NODE @  0 'NODE !  DO,  POSTPONE BEGIN ; IMMEDIATE
: ?DO   'NODE @  0 'NODE !  POSTPONE 2DUP DO,  POSTPONE = POSTPONE IF
   POSTPONE LEAVE POSTPONE THEN  POSTPONE BEGIN ; IMMEDIATE
: LOOP   LOOP,  >NODE  'NODE !  UNLOOP, ; IMMEDIATE
: +LOOP   +LOOP,  >NODE  'NODE !  UNLOOP, ; IMMEDIATE

: CASE   0 ; IMMEDIATE
: OF   1+ >R  POSTPONE OVER POSTPONE = POSTPONE IF  POSTPONE DROP  R> ;
IMMEDIATE
: ENDOF   >R  POSTPONE ELSE  R> ; IMMEDIATE
: ENDCASE   POSTPONE DROP  0 ?DO  POSTPONE THEN  LOOP ; IMMEDIATE


\ Strings

: CMOVE   OVER + SWAP ?DO  DUP C@ I C! 1+  LOOP  DROP ;
: CMOVE>   ?DUP IF  1- TUCK  + -ROT  OVER +  DO  I C@ OVER C!  1-  -1 +LOOP
   ELSE DROP  THEN  DROP ;
: MOVE   -ROT 2DUP > IF  ROT CMOVE  ELSE ROT CMOVE>  THEN ;
: FILL   -ROT  OVER + SWAP ?DO  DUP I C!  LOOP  DROP ;
: ERASE   0 FILL ;

: C=   ( adr1 adr2 -- f )   DUP C@ 1+  OVER + SWAP ?DO  DUP C@ I C@ <> IF
   DROP FALSE UNLOOP EXIT  THEN  1+  LOOP  DROP  TRUE ;
: C0END   ( adr1 u -- adr2 )   SCRATCH SWAP  2DUP + >R  MOVE  0 R> C!
   SCRATCH ;


\ Mass storage input/output

INCLUDE Machine:fileio/fs


\ Terminal input/output #2

: SPACE   BL EMIT ;
: SPACES   0 ?DO  SPACE  LOOP ;
: TYPE   OVER + SWAP ?DO  I C@ EMIT  LOOP ;


\ Mass storage input/output #1

CREATE /FILE-BUFFER 128 ,
CREATE #FILE-BUFFERS 16 ,
CREATE FILE-BUFFER# 0 ,   \ next file buffer to use
: FIRST-FILE   LIMIT @  #FILE-BUFFERS @ /FILE-BUFFER @ * - ;
: ALLOCATE-BUFFER   ( -- adr ior )   FILE-BUFFER# @  DUP #FILE-BUFFERS @ = IF
   -1  ELSE DUP 1+ FILE-BUFFER# !  /FILE-BUFFER @ *  FIRST-FILE +  0  THEN ;
: FREE-BUFFER   ( -- ior )   FILE-BUFFER#  DUP @ 0= IF  DROP -1
   ELSE -1 SWAP +!  0  THEN ;


\ Terminal input/output #3

: ACCEPT   ( adr +n1 -- +n2 )
   >R                                \ save count
   DUP BEGIN
      KEY                            \ get key
      DUP DEL? IF                    \ if Delete
         DROP                        \ get rid of key
         2DUP < IF                   \ if there's room to delete,
            1-  DEL                  \ decrement pointer & emit DEL
         THEN
            ELSE
         DUP CR? IF                  \ if end of line then
            R> 2DROP                 \ drop max. length & character,
            BL EMIT                  \ emit a space
            >-<                      \ get the length of the input
            EXIT                     \ and exit
               ELSE
            -ROT  2DUP >-<           \ if another key, check there's
            R@ < IF                  \ room for it,
               ROT DUP EMIT          \ print it,
               OVER C!               \ store it
               1+                    \ and increment the pointer
                  ELSE
               ROT DROP              \ otherwise drop the character
            THEN
         THEN
      THEN
   AGAIN ;

VARIABLE >IN

VARIABLE #TIB
: TIB   R0 @ ;

VARIABLE #FIB
CREATE FIB 0 ,

CREATE SOURCE-ID 0 ,
: SOURCE   SOURCE-ID @ IF  FIB @  #FIB @  ELSE TIB  #TIB @  THEN ;

( SAVE-INPUT returns the current input source immediately under the number of
  items returned, encoded as:
        0 = user input device, otherwise file handle )
: SAVE-INPUT   ( -- xn...x1 n )
   >IN @                             \ get >IN
   SOURCE-ID @ DUP >R IF             \ look at SOURCE-ID
      FIB @ #FIB @ 4                 \ if a file leave >IN FIB #FIB fid 2
         ELSE
      2                              \ if not leave >IN
   THEN
   R> SWAP ;                         \ get SOURCE-ID
( RESTORE-INPUT always succeeds unless the input source buffer being restored
  has been altered, which it has no way of telling. )
: RESTORE-INPUT   ( xn...x1 n -- )
   DROP
   DUP  SOURCE-ID !
   IF   #FIB !  FIB !  THEN
   >IN !
   FALSE ;

VARIABLE 'RETURN
: SAVE-INPUT>R   \ save input specification to return stack
   R> 'RETURN !                      \ save return address
   SAVE-INPUT                        \ get input specification
   DUP                               \ push it to return stack
   BEGIN  ?DUP WHILE
      ROT >R
      1-
   REPEAT
   >R
   'RETURN @ >R ;                    \ restore return address
: R>RESTORE-INPUT   \ restore input specification from return stack
   R> 'RETURN !                      \ save return address
   R> DUP                            \ pop input specification
   BEGIN  ?DUP WHILE                 \ from return stack
      R> -ROT
      1-
   REPEAT
   RESTORE-INPUT                     \ set input specification
   'RETURN @ >R ;                    \ restore return address

: PARSE   ( char -- adr u )
   >R                                \ save delimiter
   SOURCE                            \ get input source specifier
   TUCK                              \ save length of input source
   OVER +                            \ end of input source + 1
   SWAP >IN @ +                      \ start of parse area
   TUCK                              \ save start of string
   BEGIN
      2DUP > DUP IF                  \ if not at end of parse area
         DROP                        \ get rid of true flag
         DUP C@ R@ <>                \ test that delimiter not found
      THEN
      WHILE                          \ while tests above succeed
      1+                             \ look at next character
   REPEAT
   R> DROP                           \ drop delimiter
   NIP                               \ drop end of input source + 1
   OVER -                            \ address and count of string
   ROT                               \ get length of input source
   OVER >IN TUCK @ + 1+              \ calculate new value of >IN
   ROT MIN                           \ ensure it doesn't overrun
   SWAP ! ;                          \ update >IN

: WORD   ( char -- adr )
   >R                                \ save delimiter
   SOURCE                            \ get input source specifier
   OVER +                            \ end of input source + 1
   SWAP >IN @ +                      \ start of parse area
   TUCK                              \ save start of string
   BEGIN
      2DUP > DUP IF                  \ if not at end of parse area
         DROP                        \ get rid of true flag
         DUP C@ R@ =                 \ test that delimiter is found
      THEN
      WHILE                          \ while tests above succeed
      1+                             \ increment address
   REPEAT
   NIP                               \ drop end of input source + 1
   >-< >IN +!                        \ update >IN
   R> PARSE                          \ get the delimited string
   TOKEN  2DUP C!                    \ store count
   1+  2DUP +  BL SWAP C!            \ store blank at end of string
   SWAP MOVE                         \ store string
   TOKEN ;                           \ leave the string's address

: .(   [CHAR] ) PARSE TYPE ; IMMEDIATE


\ Compiler #3

: ",   ( adr u -- )   DUP C,  HERE SWAP  DUP ALLOT  MOVE ;
: SLITERAL   POSTPONE (S")  ", ALIGN ; IMMEDIATE

: S"   [CHAR] " PARSE  STATE @ IF  POSTPONE SLITERAL  ELSE S"B SWAP 2DUP 2>R
   MOVE  2R>  THEN ; IMMEDIATE

: ."   POSTPONE S"  POSTPONE TYPE ; IMMEDIATE

: CHAR   BL WORD  1+ C@ ;
: [CHAR]   CHAR  POSTPONE LITERAL ; IMMEDIATE


\ Interpreter #3

: ABORT"   POSTPONE S"  POSTPONE (ABORT") ; IMMEDIATE
: ERROR"   POSTPONE S"  POSTPONE (ERROR") ; IMMEDIATE


\ Numeric conversion

VARIABLE BASE
VARIABLE HELD

: DECIMAL   10 BASE ! ;
: HEX   16 BASE ! ;
: HOLD   -1 HELD +!  HELD @ C! ;
: SIGN   0< IF  [CHAR] - HOLD  THEN ;
: <#   TOKEN HELD ! ;
: #>   DROP  HELD @  TOKEN OVER - ;
: #   BASE @ U/MOD SWAP  DUP 10 < IF  [CHAR] 0 +
   ELSE [ CHAR A 10 - ] LITERAL +  THEN  HOLD ;
: #S   BEGIN  #  DUP 0= UNTIL ;

: .R   SWAP DUP ABS  <# #S SWAP SIGN #>  ROT OVER -  0 MAX SPACES  TYPE ;
: .   0 .R  SPACE ;

: >NUMBER   ( u1 adr1 u2 -- u3 adr2 u4 )
   OVER + SWAP                       \ form limits for a loop
   TUCK  OVER >R                     \ save initial address and
                                     \ address of last character + 1
   ?DO
      C@                             \ get next character
      DUP [CHAR] A < IF              \ convert to a digit
         [CHAR] 0  ELSE [ CHAR A 10 - ] LITERAL
      THEN
      -
      DUP  BASE @ < INVERT           \ if digit is too large...
      OVER 0<  OR IF                 \ or too small
         DROP I  LEAVE               \ leave address of character
      THEN                           \ and exit the loop
      SWAP  BASE @ *  +              \ shift number and add new digit
      I 1+                           \ address of next character
   LOOP
   DUP R> >-< ;                      \ construct u4

: NUMBER   ( adr -- n )
   0                                 \ make accumulator for >NUMBER
   OVER COUNT                        \ count the string
   OVER C@                           \ get the first character
   [CHAR] - =  DUP >R  IF            \ skip first character if it's
      1- SWAP  1+ SWAP               \ a minus and save the flag
   THEN
   >NUMBER NIP                       \ convert up to non-digit
   IF                                \ if the string's not finished,
      R> 2DROP ERROR" ?"             \ complain
   THEN
   NIP
   R> IF  NEGATE  THEN ;             \ if leading minus, negate no.


\ Compiler #4

CREATE #THREADS 1024 ,
: THREAD   ( adr -- 'thread )   @  DUP 7 RSHIFT  DUP 7 RSHIFT  XOR XOR
   #THREADS @ 1- AND  CELLS  'THREADS @ + ;

: FIND   ( adr -- adr 0 | xt 1 | xt -1 )
   DUP THREAD                        \ get address of thread
   BEGIN  @  ?DUP WHILE              \ for all words in thread
      2DUP >NAME C= IF               \ if the name matches
         DUP >INFO @                 \ and the word is not SMUDGEd
         [ HEX ] 20000000 [ DECIMAL ] AND 0= IF
            NIP                      \ drop name
            DUP >INFO @ 0< 2* INVERT \ get immediacy flag
            EXIT                     \ and exit
         THEN
      THEN
      >THREAD                        \ get next thread field
   REPEAT
   0 ;                               \ set flag to 0

: POSTPONE   BL WORD FIND  ?DUP 0= IF  ERROR" ?"  THEN  0> IF  COMPILE,
   ELSE POSTPONE (POSTPONE) ALIGN  ,  THEN ; IMMEDIATE

: HEADER   ( adr -- )
   ALIGN                             \ align DP for new definition
   DUP C@ 31 MIN                     \ get name (max. 31 chars)
   2DUP SWAP C!
   OVER FIND NIP IF                  \ check name is unique
      OVER COUNT TYPE ."  is not unique "
   THEN
   DUP >R                            \ save length
   1+                                \ length of name field
   HERE TUCK >R                      \ beginning of name field
   DUP ALLOT                         \ reserve space for name field
   MOVE                              \ write name in name field
   BL C, BL C, BL C,                 \ pad the name with spaces
   -3 ALLOT ALIGN                    \ up to the next word boundary
   LAST ,                            \ store link to last word in list
   R> THREAD  DUP @ ,                \ store link to last word in thread
   R> 24 LSHIFT  ,                   \ save length of name field
   HERE TUCK  CONTEXT !  ! ;         \ update thread and word list


\ Interpreter #4

: INTERPRET
   BEGIN  BL WORD  DUP C@ WHILE      \ while text in input stream
      FIND                           \ search for word
      STATE @  OVER IF               \ if word found in dictionary
         NEGATE = IF                 \ if compiling, and word is
            COMPILE,                 \ non-immediate, compile it
               ELSE
            EXECUTE                  \ otherwise execute it
         THEN
            ELSE                     \ if word is not found
         >R DROP                     \ drop found flag
         NUMBER                      \ try getting a number
         R> IF                       \ compile if STATE is non-zero
            POSTPONE LITERAL
         THEN
      THEN
   REPEAT DROP ;                     \ get rid of input address

: REFILL   ( -- f )
   SOURCE-ID @ IF                    \ switch on SOURCE-ID
      FIB @ 128 R@ READ-LINE         \ if a file, read a line
      ABORT" file read error during REFILL"
                                     \ if an exception occurred, abort
      SWAP #FIB !  0 >IN !           \ set no. of chars in line
         ELSE
      TIB 80 ACCEPT                  \ else get a line of text to TIB
      #TIB !  0 >IN !  TRUE
   THEN ;

: ?STACK   DEPTH 0< ABORT" stack underflow" ;
: (QUIT)
   POSTPONE [  R0 @ RP!
   0 SOURCE-ID !
   BEGIN  CR REFILL WHILE
      INTERPRET  ?STACK  STATE @ 0= IF  ." ok"  THEN
   REPEAT
   ." parse area empty" ABORT ;


\ Compiler #5

: :   BL WORD HEADER  TRUE SMUDGE  LINK,  ] ;
: ;   UNLINK,  POSTPONE [  FALSE SMUDGE  ALIGN ; IMMEDIATE


\ Miscellaneous

: (
   BEGIN
      [CHAR] ) PARSE NIP             \ parse up to ) or end of area
      SOURCE-ID @ 0= IF              \ exit if not reading from file
         DROP EXIT
      THEN
      0<> IF                         \ is parse area empty?
         SOURCE DROP >IN @ 1- +  C@ [CHAR] ) <>
                                     \ if not, was last character )?
            ELSE
         TRUE                        \ if empty we must refill
      THEN
      WHILE                          \ if parse area empty or no )
      REFILL 0=                      \ found, refill and parse again
   UNTIL THEN ;
IMMEDIATE

: \   SOURCE NIP >IN ! ; IMMEDIATE
: .S   ?STACK  DEPTH ?DUP IF  1- 0 SWAP DO  I PICK .  -1 +LOOP
   ELSE ." stack empty "  THEN ;


\ Mass storage input/output #3

: INCLUDE-FILE   ( i*x fid -- j*x )
   SAVE-INPUT>R                      \ save current input source
   SOURCE-ID !                       \ set up new input source
   ALLOCATE-BUFFER  ABORT" no more file buffers"
                                     \ allocate new file buffer
   FIB !
   BEGIN  REFILL WHILE               \ interpret the file
      INTERPRET
   REPEAT
   FREE-BUFFER  ABORT" no file buffer to free"
                                     \ free the file buffer
   R>RESTORE-INPUT ;                 \ restore the input source
: INCLUDED   ( i*x adr u -- j*x )
   2DUP  R/O OPEN-FILE IF            \ open file; if error,
      DROP                           \ get rid of bad fid
      TYPE ."  can't be INCLUDED"    \ give error message
      ABORT                          \ and abort
   THEN
   >R                                \ save fid
   2DROP                             \ drop adr u
   R@ INCLUDE-FILE                   \ include the file
   R> CLOSE-FILE                     \ close the file; if error,
   ABORT" error after INCLUDEing" ;  \ give error message and abort


\ Compiler #6

: '   BL WORD FIND  0= IF  ERROR" ?"  THEN ;
: [']   '  POSTPONE LITERAL ; IMMEDIATE


\ Defining

: CREATE   BL WORD HEADER  CREATE, ;
: VARIABLE   CREATE  CELL ALLOT ;


\ Word lists

VARIABLE CURSORX   \ cursor x position during WORDS
: WRAP?   ( -- f )   CURSORX @ + WIDTH < INVERT ;
: NEWLINE   0 CURSORX !  CR ;
: WORDS
   NEWLINE                           \ start listing on a new line
   CONTEXT @                         \ get start of chain
   BEGIN  >LINK @ ?DUP WHILE         \ for each word in the chain
      DUP >NAME COUNT                \ get the name
      DUP WRAP? IF  NEWLINE  THEN    \ new line if necessary
      DUP CURSORX +!                 \ advance the cursor
      TYPE                           \ type the name
      3 WRAP? IF                     \ leave a gap or move to a new
         NEWLINE                     \ line
            ELSE
         3 SPACES  3 CURSORX +!
      THEN
   REPEAT
   CURSORX @ 0<> IF  NEWLINE  THEN ; \ ensure we're on a new line


\ Exceptions

: (THROW)
   ?DUP IF
      CASE
         -1 OF  S0 @ SP!  QUIT  ENDOF
         -2 OF  TYPE ABORT  ENDOF
         -10 OF  ." division by zero"  ABORT  ENDOF
         -11 OF  ." quotient too large"  ABORT  ENDOF
         -56 OF  (QUIT)  ENDOF
         -512 OF  ROT COUNT TYPE  SPACE  TYPE  ABORT  ENDOF
         ." exception " DUP . ." raised"  ABORT
      ENDCASE
   THEN ;


\ Initialisation and version number

CREATE VERSION 2 ,
: %.   <# # # [CHAR] . HOLD #S #>  TYPE ;

: START
   LIMIT @                           \ get address of end of memory
   #FILE-BUFFERS @ /FILE-BUFFER @ * -
                                     \ file buffers,
   256 5 * -                         \ and TIB, PAD, TOKEN, SCRATCH and S"B
   DUP R0 !  DUP RP!                 \ set R0 and RP
   4096 CELLS -                      \ make room for return stack
   DUP S0 !  SP!                     \ set S0 and SP
   [ ' (THROW) <M0 ] LITERAL 'THROW @ !
                                     \ set up the 'THROW vector
   R0 @  LIMIT @  OVER -  ERASE      \ erase buffers
   DECIMAL                           \ numbers treated as base 10
   CR ." mForth v" VERSION @ %.  ."  2nd April 1997"
   CR ." (c) Reuben Thomas 1991-97" CR
                                     \ display the start message
   QUIT ;                            \ enter the main loop