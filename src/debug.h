/* DEBUG.H

    (c) Reuben Thomas 1994-2018

    Header for debug.c.

*/


#ifndef BEETLE_DEBUG
#define BEETLE_DEBUG


#include "beetle.h"     /* main header */


extern int instrs;  /* number of instructions assembled */
extern int ibytes;  /* number of opcodes assembled in current instruction word
                       so far */
extern UCELL current;	/* where the current instruction word will be stored */
extern UCELL here;  /* where we assemble the next instruction word or literal */
extern UCELL S0;    /* address of base of data stack */
extern UCELL R0;    /* address of base of return stack */

void ass(BYTE instr);	/* assemble an instruction */
void lit(CELL literal);	/* assemble a cell literal */
void ilit(CELL literal);    /* assemble an immediate literal */
void plit(void (*literal)(void));  /* assemble a machine-dependent function pointer literal,
                                      including the relevant LITERAL instructions. */
void start_ass(void);	/* start assembly, initialising variables */
void end_ass(void);  /* end assembly, storing any pending instructions */
const char *disass(BYTE opcode);  /* disassemble an instruction */
BYTE toass(char *token);    /* convert a instruction to its opcode */

char *val_data_stack(void); /* return the current data stack as a string */
void show_data_stack(void); /* show the current contents of the data stack */
void show_return_stack(void);	/* show the current contents of the return stack */


#endif
