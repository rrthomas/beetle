/* DEBUG.H

    (c) Reuben Thomas 1994-1995

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
