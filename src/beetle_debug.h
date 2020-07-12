// VM debugging functions
// These are undocumented and subject to change.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#ifndef beetle_BEETLE_DEBUG
#define beetle_BEETLE_DEBUG


int beetle_byte_size(beetle_CELL v); // return number of significant bytes in a beetle_CELL quantity

void beetle_ass(beetle_BYTE instr);	// assemble an instruction
void beetle_lit(beetle_CELL literal);	// assemble a cell literal
bool beetle_ilit(beetle_CELL literal);    // assemble an immediate literal, returning false if it doesn't fit
void beetle_plit(void (*literal)(void));  // assemble a machine-dependent function pointer literal,
                                   // including the relevant LITERAL instructions
void beetle_start_ass(beetle_UCELL addr);	// start assembly at the given address
beetle_UCELL beetle_ass_current(void);	// return address of beetle_CELL currently being assembled to
const char *beetle_disass(beetle_BYTE opcode);  // disassemble an instruction
beetle_BYTE beetle_toass(const char *token);    // convert a instruction to its opcode

char *beetle_val_data_stack(void); // return the current data stack as a string
void beetle_show_data_stack(void); // show the current contents of the data stack
void beetle_show_return_stack(void);	// show the current contents of the return stack


#endif
