// Header for debug.c.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef BEETLE_DEBUG
#define BEETLE_DEBUG


#include "beetle.h"     // main header


int byte_size(CELL v); // return number of significant bytes in a CELL quantity

void ass(BYTE instr);	// assemble an instruction
void lit(CELL literal);	// assemble a cell literal
bool ilit(CELL literal);    // assemble an immediate literal, returning false if it doesn't fit
void plit(void (*literal)(void));  // assemble a machine-dependent function pointer literal,
                                   // including the relevant LITERAL instructions
void start_ass(UCELL addr);	// start assembly, initialising variables
const char *disass(BYTE opcode);  // disassemble an instruction
BYTE toass(const char *token);    // convert a instruction to its opcode

char *val_data_stack(void); // return the current data stack as a string
void show_data_stack(void); // show the current contents of the data stack
void show_return_stack(void);	// show the current contents of the return stack


#endif
