/* DEBUG.H

    Vrsn  Date   Comment
    ----|-------|---------------------------------------------------------------
    0.00 17nov94
    0.01 18nov94 Changed showEP to val_EP. Added endass.
    0.02 21nov94 Added lit.
    0.03 22nov94 Added ibytes and current; added start_ass, renamed endass
                 to end_ass, and added ilit.
    0.04 28nov94 Added R0 and show_return_stack.
    0.05 29nov94 Added val_data_stack.
    0.06 19feb95 Added toass. Return type of disass changed to const char *.

    Reuben Thomas


    Header for debug.c.

*/


#ifndef BEETLE_DEBUG
#define BEETLE_DEBUG


#include "beetle.h"     /* main header */


extern int instrs;  /* number of instructions assembled */
extern int ibytes;  /* number of opcodes assembled in current instruction word
                       so far */
extern CELL *current;	/* where the current instruction word will be stored */
extern CELL *here;  /* where we assemble the next instruction word or literal */
extern CELL *S0;    /* pointer to base of data stack */
extern CELL *R0;    /* pointer to base of return stack */

void ass(BYTE instr);	/* assemble an instruction */
void lit(CELL literal);	/* assemble a cell literal */
void ilit(CELL literal);    /* assemble an immediate literal */
void start_ass(void);	/* start assembly, initialising variables */
void end_ass(void);  /* end assembly, storing any pending instructions */
const char *disass(BYTE opcode);  /* disassemble an instruction */
BYTE toass(char *token);    /* convert a instruction to its opcode */

CELL val_EP(void);  /* return the current contents of EP */
char *val_data_stack(void); /* return the current data stack as a string */
void show_data_stack(void); /* show the current contents of the data stack */
void show_return_stack(void);	/* show the current contents of the return stack */


#endif
