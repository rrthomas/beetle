REM >ARMBeetle
REM ARM version of Beetle
REM Reuben Thomas   18/5-27/6/96, 2/12/98

REM Reserve space for code
code_size%=12*1024
DIM code% code_size%-1
L%=code%+code_size%

REM Registers
a1=0:a2=1:MEMORY=12:EP=9:RP=8:BSP=7:A=6:M0=5:base=4

REM Assemble and save images
CHECKED=TRUE

name$="run"
stepping%=FALSE
PROCassemble

name$="step"
stepping%=TRUE
PROCassemble

END


DEFPROCassemble
LOCAL pass%,i%
FOR pass%=%10101000 TO %10101010 STEP 2
P%=code%
[               opt     pass%
                area    "ARM$$Code","CODE, READONLY"

; Perform Beetle's execution loop
; on entry- no parameters
; on exit - a1 = reason code

                import  "MEMORY"
                import  "M0"
                import  "A"
                import  "SP"
                import  "RP"
                import  "EP"
                import  "THROW"
                import  "BAD"
                import  "ADDRESS"
                import  "lib"
.bregs          dcd     MEMORY@,M0@,A@,SP@,RP@,EP@,THROW@,BAD@,ADDRESS@
.lrdump         dcd     0
]
IF stepping% THEN
[               opt     pass%
.single_step
                export  "single_step"
]
ELSE
[               opt     pass%
.run
                export  "run"
]
ENDIF
[               opt     pass%
                str     lr,lrdump
                bl      get_regs
                b       cont            ; start executing

.get_regs       stmfd   sp!,{v1-v6,sl}
                adr     a1,bregs        ; load Beetle's registers
                ldmia   a1,{a1,M0,A,BSP,RP,EP}
                ldr     M0,[M0]
                ldr     A,[A]
                ldr     BSP,[BSP]
                ldr     RP,[RP]
                ldr     EP,[EP]
                ldr     MEMORY,[a1]
                adr     base,table      ; make base point to the jump table
                movs    pc,lr

; Exit, returning reason code and registers

.stop           ldr     lr,lrdump
.restore_regs   adr     a2,bregs+8
                ldmia   a2!,{a3-a4,v1-v2,ip}
                str     A,[a3]
                str     BSP,[a4]
                str     RP,[v1]
                str     EP,[v2]
                ldr     v1,[M0],#8
                str     v1,[ip]
                ldmia   a2!,{a2-a3}
                ldmia   M0,{v1,v3}
                str     v1,[a2]
                str     v3,[a3]
                ldmfd   sp!,{v1-v6,sl}
                movs    pc,lr

; Branch table
; Contains the address of each instruction's code from 00h to FFh. The undefined
; instructions all call the undefined instruction trap code.

.table          dcd     next,dup,drop,swap
                dcd     over,rot,nrot,tuck
                dcd     nip,pick,roll,qdup
                dcd     tor,rfrom,rfetch,less
                dcd     greater,equal,nequal,less0
                dcd     greater0,equal0,nequal0,uless
                dcd     ugreater,zero,one,mone
                dcd     cell,mcell,plus,minus
                dcd     swapminus,plus1,minus1,pluscell
                dcd     minuscell,star,slash,mod
                dcd     slashmod,uslashmod,sslashrem,slash2
                dcd     cells,abs,negate,max
                dcd     min,invert,and,or
                dcd     xor,lshift,rshift,lshift1
                dcd     rshift1,fetch,store,cfetch
                dcd     cstore,pstore,spfetch,spstore
                dcd     rpfetch,rpstore,branch,branchi
                dcd     qbranch,qbranchi,execute,fexecute
                dcd     call,calli,exit,do
                dcd     loop,loopi,ploop,ploopi
                dcd     unloop,j,literal,literali
                dcd     throw,halt,create,lib
                dcd     nop,nop,nop,nop
                ; os, link, run and step not implemented
]
FOR i%=&5C TO &FE
[               opt     pass%
                dcd     illegal
]
NEXT
[               opt     pass%
                dcd     next

; Instruction code, in opcode order

; next is part of branch

.dup            FNchecksp(BSP,-4,0)
                ldr     a1,[BSP],#-4
                str     a1,[BSP]
                FNcont("AL")

.drop           add     BSP,BSP,#4
                FNcont("AL")

.swap           FNchecksp(BSP,0,4)
                ldmia   BSP,{a1-a2}
                mov     a3,a1
                stmia   BSP,{a2-a3}
                FNcont("AL")

.over           FNchecksp(BSP,-4,4)
                ldr     a1,[BSP,#4]
                str     a1,[BSP,#-4]!
                FNcont("AL")

.rot            FNchecksp(BSP,0,8)
                ldmia   BSP,{a1-a3}
                str     a3,[BSP]
                stmib   BSP,{a1-a2}
                FNcont("AL")

.nrot           FNchecksp(BSP,0,8)
                ldmia   BSP,{a1-a3}
                str     a1,[BSP,#8]
                stmia   BSP,{a2-a3}
                FNcont("AL")

.tuck           FNchecksp(BSP,-4,4)
                ldmia   BSP,{a1-a2}
                mov     a3,a1
                sub     BSP,BSP,#4
                stmia   BSP,{a1-a3}
                FNcont("AL")

.nip            FNchecksp(BSP,0,4)
                ldr     a1,[BSP]
                str     a1,[BSP,#4]!
                FNcont("AL")

.pick           FNcheckp(BSP,0,a1)
]
IF CHECKED THEN
[               opt     pass%
                ldr     a1,[BSP]
                add     a2,BSP,a1,asl#2
                FNcheckp(1,4,a2)
                add     BSP,BSP,#4
]
ELSE
[               opt     pass%
                ldr     a1,[BSP],#4
]
ENDIF
[               opt     pass%
                ldr     a1,[BSP,a1,asl#2]
                str     a1,[BSP,#-4]!
                FNcont("AL")

.roll
]
IF CHECKED THEN
[               opt     pass%
                ldr     a1,[BSP]
                add     a2,BSP,a1,asl#2
                FNcheckp(1,4,a2)
                add     BSP,BSP,#4
]
ELSE
[               opt     pass%
                ldr     a1,[BSP],#4
]
ENDIF
[               opt     pass%
                add     a1,BSP,a1,asl#2
                ldr     a2,[a1]
.roll_loop      ldr     a3,[a1,#-4]!
                str     a3,[a1,#4]
                cmp     a1,BSP
                bhs     roll_loop
                str     a2,[BSP]
                FNcont("AL")

.qdup           FNchecksp(BSP,-4,0)
                ldr     a1,[BSP]
                cmp     a1,#0
                strne   a1,[BSP,#-4]!
                FNcont("AL")

.tor            FNcheckp(RP,-4,a1)
                FNcheckp(BSP,0,a1)
                ldr     a1,[BSP],#4
                str     a1,[RP,#-4]!
                FNcont("AL")

.rfrom          FNcheckp(BSP,-4,a1)
                FNcheckp(RP,0,a1)
                ldr     a1,[RP],#4
                str     a1,[BSP,#-4]!
                FNcont("AL")

.rfetch         FNcheckp(BSP,-4,a1)
                FNcheckp(RP,0,a1)
                ldr     a1,[RP]
                str     a1,[BSP,#-4]!
                FNcont("AL")

.less           FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a2,a1
                mvnlt   a1,#0
                movge   a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.greater        FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a2,a1
                mvngt   a1,#0
                movle   a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.equal          FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a1,a2
                mvneq   a1,#0
                movne   a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.nequal         FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                eors    a1,a1,a2
                mvnne   a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.less0          FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                mov     a1,a1,asr#31
                str     a1,[BSP]
                FNcont("AL")

.greater0       FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                rsb     a1,a1,#0
                mov     a1,a1,asr#31
                str     a1,[BSP]
                FNcont("AL")

.equal0         FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                cmp     a1,#0
                mvneq   a1,#0
                movne   a1,#0
                str     a1,[BSP]
                FNcont("AL")

.nequal0        FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                cmp     a1,#0
                mvnne   a1,#0
                str     a1,[BSP]
                FNcont("AL")

.uless          FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a2,a1
                mvnlo   a1,#0
                movhs   a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.ugreater       FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a2,a1
                mvnhi   a1,#0
                movls   a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.zero           FNcheckp(BSP,-4,a1)
                mov     a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.one            FNcheckp(BSP,-4,a1)
                mov     a1,#1
                str     a1,[BSP,#-4]!
                FNcont("AL")

.mone           FNcheckp(BSP,-4,a1)
                mvn     a1,#0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.cell           FNcheckp(BSP,-4,a1)
                mov     a1,#4
                str     a1,[BSP,#-4]!
                FNcont("AL")

.mcell          FNcheckp(BSP,-4,a1)
                mvn     a1,#(4-1)
                str     a1,[BSP,#-4]!
                FNcont("AL")

.plus           FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                add     a1,a2,a1
                str     a1,[BSP,#-4]!
                FNcont("AL")

.minus          FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                sub     a1,a2,a1
                str     a1,[BSP,#-4]!
                FNcont("AL")

.swapminus      FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                sub     a1,a1,a2
                str     a1,[BSP,#-4]!
                FNcont("AL")

.plus1          FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                add     a1,a1,#1
                str     a1,[BSP]
                FNcont("AL")

.minus1         FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                sub     a1,a1,#1
                str     a1,[BSP]
                FNcont("AL")

.pluscell       FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                add     a1,a1,#4
                str     a1,[BSP]
                FNcont("AL")

.minuscell      FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                sub     a1,a1,#4
                str     a1,[BSP]
                FNcont("AL")

.star           FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                mul     a1,a2,a1
                str     a1,[BSP,#-4]!
                FNcont("AL")

; Division routines

.divbyzero      mvn     a1,#9           ; throw code -10
                str     a1,[BSP,#-12]!  ; restore division quantities to stack
                b       throw

; Perform an unsigned division
; on entry- a1 = divisor, a2 = dividend
; on exit - a1 = quotient, a2 = remainder, a3 = divisor, a4 = corrupted
.divide         movs    a3,a1           ; copy divisor
                beq     divbyzero       ; abort on divide by zero
                mov     a4,a1           ; copy divisor (need a1 for quotient)
                cmp     a4,a2,lsr#16    ; shift divisor left until MSB in same
                movls   a4,a4,lsl#16    ; position as dividend's
                cmp     a4,a2,lsr#8
                movls   a4,a4,lsl#8
                cmp     a4,a2,lsr#4
                movls   a4,a4,lsl#4
                cmp     a4,a2,lsr#2
                movls   a4,a4,lsl#2
                cmp     a4,a2,lsr#1
                movls   a4,a4,lsl#1
                mov     a1,#0           ; quotient=0
.divide_loop    cmp     a2,a4           ; if dividend>divisor
                subhs   a2,a2,a4        ; then dividend-=divisor
                adc     a1,a1,a1        ; shift quotient and add carry
                mov     a4,a4,lsr#1     ; shift divisor
                cmp     a4,a3           ; continue until divisor<original
                bhs     divide_loop
                movs    pc,lr

; Perform a signed floored division
; on entry- stack = ( dividend divisor )
; on exit - a1 = quotient, a2 remainder, a3 = divisor, a4 = corrupted
.sdivide        ldmfd   BSP !,{a1-a2}
                stmfd   sp!,{v1,lr}
                mov     v1,lr           ; save link
                eor     v1,a1,a2        ; get sign of quotient
                and     v1,v1,#1<<31
                add     v1,v1,a1,lsr#1  ; get sign of remainder (=divisor)
                cmp     a1,#0           ; ensure divisor is positive
                rsbmi   a1,a1,#0
                cmp     a2,#0           ; ensure dividend is positive
                rsbmi   a2,a2,#0
                bl      divide
                tst     v1,#1<<31       ; replace sign on quotient
                rsbne   a1,a1,#0
                cmpne   a2,#0           ; if the quotient is negative and
                subne   a1,a1,#1        ; remainder non-zero then floor the
                subne   a2,a3,a2        ; quotient and correct the remainder
                tst     v1,#1<<30       ; replace sign on remainder
                rsbne   a2,a2,#0
                ldmfd   sp!,{v1,pc}

.slash          FNchecksp(BSP,0,4)
                bl      sdivide
                str     a1,[BSP,#-4]!
                FNcont("AL")

.mod            FNchecksp(BSP,0,4)
                bl      sdivide
                str     a2,[BSP,#-4]!
                FNcont("AL")

.slashmod       FNchecksp(BSP,0,4)
                bl      sdivide
                stmfd   BSP !,{a1-a2}
                FNcont("AL")

.uslashmod      FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                bl      divide
                stmfd   BSP !,{a1-a2}
                FNcont("AL")

.sslashrem      FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                stmfd   sp!,{v1}
                eor     v1,a1,a2        ; get sign of quotient
                and     v1,v1,#1<<31
                add     v1,v1,a2,lsr#1  ; get sign of remainder (=dividend)
                cmp     a1,#0           ; ensure divisor is positive
                rsbmi   a1,a1,#0
                cmp     a2,#0           ; ensure dividend is positive
                rsbmi   a2,a2,#0
                bl      divide
                tst     v1,#1<<31       ; replace sign on quotient
                rsbne   a1,a1,#0
                tst     v1,#1<<30       ; replace sign on remainder
                rsbne   a2,a2,#0
                stmfd   BSP !,{a1-a2}
                ldmfd   sp!,{v1}
                FNcont("AL")

.slash2         FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                mov     a1,a1,asr#1
                str     a1,[BSP]
                FNcont("AL")

.cells          FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                mov     a1,a1,asl#2
                str     a1,[BSP]
                FNcont("AL")

.abs            FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                cmp     a1,#0
                rsbmi   a1,a1,#0
                str     a1,[BSP]
                FNcont("AL")

.negate         FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                rsb     a1,a1,#0
                str     a1,[BSP]
                FNcont("AL")

.max            FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a1,a2
                movlt   a1,a2
                str     a1,[BSP,#-4]!
                FNcont("AL")

.min            FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                cmp     a1,a2
                movgt   a1,a2
                str     a1,[BSP,#-4]!
                FNcont("AL")

.invert         FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                mvn     a1,a1
                str     a1,[BSP]
                FNcont("AL")

.and            FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                and     a1,a1,a2
                str     a1,[BSP,#-4]!
                FNcont("AL")

.or             FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                orr     a1,a1,a2
                str     a1,[BSP,#-4]!
                FNcont("AL")

.xor            FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                eor     a1,a1,a2
                str     a1,[BSP,#-4]!
                FNcont("AL")

.lshift         FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                mov     a2,a2,lsl a1
                str     a2,[BSP,#-4]!
                FNcont("AL")

.rshift         FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                mov     a2,a2,lsr a1
                str     a2,[BSP,#-4]!
                FNcont("AL")

.lshift1        FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                mov     a1,a1,lsl#1
                str     a1,[BSP]
                FNcont("AL")

.rshift1        FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                mov     a1,a1,lsr#1
                str     a1,[BSP]
                FNcont("AL")

.fetch          FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                FNchecka(a1)
                ldr     a1,[a1,M0]
                str     a1,[BSP]
                FNcont("AL")

.store          FNchecksp(BSP,0,4)
]
IF CHECKED THEN
[               opt     pass%
                ldmfd   BSP,{a1-a2}
                FNchecka(a1)
                add     BSP,BSP,#8
]
ELSE
[               opt     pass%
                ldmfd   BSP !,{a1-a2}
]
ENDIF
[               opt     pass%
                str     a2,[a1,M0]
                FNcont("AL")

.cfetch         FNcheckp(BSP,0,a1)
                ldr     a1,[BSP]
                FNcheck(a1)
                ldrb    a1,[a1,M0]
                str     a1,[BSP]
                FNcont("AL")

.cstore         FNchecksp(BSP,0,4)
]
IF CHECKED THEN
[               opt     pass%
                ldmfd   BSP,{a1-a2}
                FNcheck(a1)
                add     BSP,BSP,#8
]
ELSE
[               opt     pass%
                ldmfd   BSP !,{a1-a2}
]
ENDIF
[               opt     pass%
                strb    a2,[a1,M0]
                FNcont("AL")

.pstore         FNchecksp(BSP,0,4)
]
IF CHECKED THEN
[               opt     pass%
                ldmfd   BSP,{a1-a2}
                FNcheck(a1)
                add     BSP,BSP,#8
]
ELSE
[               opt     pass%
                ldmfd   BSP !,{a1-a2}
]
ENDIF
[               opt     pass%
                ldr     a3,[a1,M0]
                add     a3,a3,a2
                str     a3,[a1,M0]
                FNcont("AL")

.spfetch        FNcheckp(BSP,-4,a1)
                sub     a1,BSP,M0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.spstore        FNcheckp(BSP,0,a1)
                ldr     BSP,[BSP]
                add     BSP,BSP,M0
                FNcont("AL")

.rpfetch        FNcheckp(BSP,-4,a1)
                sub     a1,RP,M0
                str     a1,[BSP,#-4]!
                FNcont("AL")

.rpstore        FNcheckp(BSP,0,a1)
                ldr     RP,[BSP],#4
                add     RP,RP,M0
                FNcont("AL")

.branch         FNcheckp(EP,0,a1)
                ldr     EP,[EP]
                add     EP,EP,M0
.next           FNnext("AL")
.cont           FNcont("AL")

.branchi        add     EP,EP,A,asl#2
                FNnext("AL")
                FNcont("AL")

.qbranch        FNcheckp(BSP,0,a1)
                FNcheckp(EP,0,a1)
                ldr     a1,[BSP],#4
                cmp     a1,#0
                ldreq   EP,[EP]
                addeq   EP,EP,M0
                FNnext("eq")
                addne   EP,EP,#4
                FNcont("AL")

.qbranchi       FNcheckp(BSP,0,a1)
                ldr     a1,[BSP],#4
                cmp     a1,#0
                addeq   EP,EP,A,asl#2
                FNnext("AL")
                FNcont("AL")

.execute        FNcheckp(RP,-4,a1)
                FNcheckp(BSP,0,a1)
                sub     EP,EP,M0
                str     EP,[RP,#-4]!
                ldr     EP,[BSP],#4
                add     EP,EP,M0
                FNnext("AL")
                FNcont("AL")

.fexecute       FNcheckp(RP,-4,a1)
                FNcheckp(BSP,0,a1)
]
IF CHECKED THEN
[               opt     pass%
                ldr     a1,[BSP]
                FNchecka(a1)
                add     BSP,BSP,#4
]
ELSE
[               opt     pass%
                ldr     a1,[BSP],#4
]
ENDIF
[               opt     pass%
                sub     EP,EP,M0
                str     EP,[RP,#-4]!
                ldr     EP,[a1,M0]
                add     EP,EP,M0
                FNnext("AL")
                FNcont("AL")

.call           FNcheckp(RP,-4,a1)
                FNcheckp(EP,0,a1)
                sub     a1,EP,M0
                add     a1,a1,#4
                str     a1,[RP,#-4]!
                ldr     EP,[EP]
                add     EP,EP,M0
                FNnext("AL")
                FNcont("AL")

.calli          FNcheckp(RP,-4,a1)
                sub     a1,EP,M0
                str     a1,[RP,#-4]!
                add     EP,EP,A,asl#2
                FNnext("AL")
                FNcont("AL")

.exit           FNcheckp(RP,0,a1)
                ldr     EP,[RP],#4
                add     EP,EP,M0
                FNnext("AL")
                FNcont("AL")

.do             FNchecksp(RP,-8,-4)
                FNchecksp(BSP,0,4)
                ldmfd   BSP !,{a1-a2}
                stmfd   RP !,{a1-a2}
                FNcont("AL")

.loop           FNchecksp(RP,0,4)
                FNcheckp(EP,0,a1)
                ldmfd   RP !,{a1-a2}
                add     a1,a1,#1
                cmp     a1,a2
                addeq   EP,EP,#4
                strne   a1,[RP,#-8]!
                ldrne   EP,[EP]
                addne   EP,EP,M0
                FNnext("ne")
                FNcont("AL")

.loopi          FNchecksp(RP,0,4)
                ldmfd   RP !,{a1-a2}
                add     a1,a1,#1
                cmp     a1,a2
                strne   a1,[RP,#-8]!
                addne   EP,EP,A,asl#2
                FNnext("AL")
                FNcont("AL")

.ploop          FNchecksp(RP,0,4)
                FNcheckp(BSP,0,a1)
                FNcheckp(EP,0,a1)
                ldmfd   RP !,{a1-a2}
                sub     a3,a1,a2        ; index-limit
                ldr     a4,[BSP],#4     ; offset
                add     a1,a1,a4        ; index = index+offset
                sub     a2,a1,a2        ; new index-limit
                teq     a2,a3           ; find whether signs are the same
                addmi   EP,EP,#4
                strpl   a1,[RP,#-8]!
                ldrpl   EP,[EP]
                addpl   EP,EP,M0
                FNnext("ne")
                FNcont("AL")

.ploopi         FNchecksp(RP,0,4)
                FNcheckp(BSP,0,a1)
                ldmfd   RP !,{a1-a2}
                sub     a3,a1,a2        ; index-limit
                ldr     a4,[BSP],#4     ; offset
                add     a1,a1,a4        ; index = index+offset
                sub     a2,a1,a2        ; new index-limit
                teq     a2,a3           ; find whether signs are the same
                strpl   a1,[RP,#-8]!
                addpl   EP,EP,A,asl#2
                FNnext("AL")
                FNcont("AL")

.unloop         add     RP,RP,#8
                FNcont("AL")

.j              FNcheckp(BSP,-4,a1)
                FNcheckp(RP,8,a1)
                ldr     a1,[RP,#8]
                str     a1,[BSP,#-4]!
                FNcont("AL")

.literal        FNcheckp(BSP,-4,a1)
                FNcheckp(EP,0,a1)
                ldr     a1,[EP],#4
                str     a1,[BSP,#-4]!
                FNcont("AL")

.literali       FNcheckp(BSP,-4,a1)
                str     A,[BSP,#-4]!
                FNnext("AL")
                FNcont("AL")

.throw          sub     EP,EP,M0
                str     EP,[M0,#8]
                ldr     EP,[M0]
                tst     EP,#3
                bne     throw_bad
                cmp     EP,MEMORY
                addlo   EP,EP,M0
                FNnext("lo")
                FNcont("lo")
.throw_bad      mvn     a1,#255         ; return -259
                sub     a1,a1,#3
                b       stop

.halt           FNcheckp(BSP,0,a1)
                ldr     a1,[BSP],#4
                b       stop

.create         FNcheckp(BSP,-4,a1)
                sub     a1,EP,M0
                str     a1,[BSP,#-4]!
.nop            FNcont("AL")

.lib            FNchecksp(BSP,-4,0)
                ldr     a1,[BSP],#4
                cmp     a1,#12
                bhi     lib_bad
                bl      restore_regs
                bl      lib@
                bl      get_regs
                FNcont("AL")
.lib_bad        mvn     a1,#255         ; throw code -257
                sub     a1,a1,#1
                str     a1,[BSP,#-4]!
                b       throw

.illegal        FNcheckp(BSP,-4,a1)
                mvn     a1,#255         ; throw code -256
                str     a1,[BSP,#-4]!
                b       throw
]
IF CHECKED THEN
[               opt     pass%
                mov     a1,a4
                b       unaligned
                mov     a1,a3
                b       unaligned
                mov     a1,a2
.unaligned      mvn     a2,#22          ; throw code -23
                b       raise

                mov     a1,a4
                b       invalid
                mov     a1,a3
                b       invalid
                mov     a1,a2
.invalid        mvn     a2,#8           ; throw code -9
.raise          str     a1,[M0,#12]     ; save invalid address in -ADDRESS
                sub     a1,BSP,M0       ; if BSP is unaligned or would be out of
                tst     a1,#3           ; range when another cell is pushed on
                bne     spinvalid       ; to the stack, return -258
                sub     a1,a1,#4
                cmp     a1,MEMORY
                strlo   a2,[BSP,#-4]!
                blo     throw
.spinvalid      mvn     a1,#255         ; return -258
                sub     a1,a1,#2
                b       stop

                save    name$+"/o"
]
ENDIF
NEXT
ENDPROC


DEFFNnext(cond$)
[               opt     pass%
                cond    cond$
                ldr=    A,[EP],#4       ; refill instruction accumulator
]
=pass%

DEFFNcont(cond$)
IF stepping% AND P%<>cont THEN
[               opt     pass%
                cond    cond$
                mov=    r0,#0
                b=      stop
]
ELSE
[               opt     pass%
                cond    cond$
                and=    a1,A,#255       ; get next opcode
                mov=    A,A,asr#8       ; remove opcode from A
                ldr=    pc,[base,a1,asl#2] ; look up address in table & branch
]
ENDIF
=pass%

DEFFNcheck(reg%)
IF CHECKED THEN
[               opt     pass%
                cmp     reg%,MEMORY
                bhs     invalid-reg%*8+(4 AND reg%>0)
]
ENDIF
=pass%

DEFFNchecka(reg%)
IF CHECKED THEN
[               opt     pass%
                FNcheck(reg%)
                tst     reg%,#3
                bne     unaligned+reg%*8+(4 AND reg%>0)
]
ENDIF
=pass%

DEFFNcheckp(ptr%,off%,temp%)
IF CHECKED THEN
[               opt     pass%
                sub     temp%,ptr%,M0
]
IF off%>0 THEN
[               opt     pass%
                add     temp%,temp%,#off%
]
ELSE
IF off%<0 THEN
[               opt     pass%
                sub     temp%,temp%,#-off%
]
ENDIF
ENDIF
[               opt     pass%
                FNchecka(temp%)
]
ENDIF
=pass%

DEFFNchecksp(ptr%,down%,up%)
IF CHECKED THEN
[               opt     pass%
                tst     ptr%,#3
                bne     unaligned+ptr%*8+(4 AND ptr%>0)
                sub     a1,ptr%,M0
]
IF down%<0 THEN
[               opt     pass%
                sub     a1,a1,#-down%
]
ELSE
IF down%>0 THEN
[               opt     pass%
                add     a1,a1,#down%
]
ENDIF
ENDIF
[               opt     pass%
                cmp     a1,MEMORY
                bhs     invalid
]
IF up%-down%<>0 THEN
[               opt     pass%
                add     a1,a1,#(up%-down%)
]
ENDIF
[               opt     pass%
                cmp     a1,MEMORY
                bhs     invalid
]
ENDIF
=pass%
