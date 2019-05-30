// Public data structures and interface calls specified in the VM definition.
// This is the header file to include in programs using an embedded VM.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef PACKAGE_UPPER_PACKAGE_UPPER
#define PACKAGE_UPPER_PACKAGE_UPPER


#include "config.h"

#include <stdio.h>      // for the FILE type
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


// Basic types
typedef uint8_t BYTE;
typedef int32_t CELL;
typedef uint32_t UCELL;
typedef uint64_t DUCELL;
#define CHAR_MASK ((1 << CHAR_BIT) - 1)
#define CELL_BIT (sizeof(CELL_W) * CHAR_BIT)
#define CELL_MAX (UINT32_MAX)
#define CELL_MASK CELL_MAX

// VM registers

// ENDISM is fixed at compile-time, which seems reasonable, as
// machines rarely change endianness while switched on!
#ifdef WORDS_BIGENDIAN
#define ENDISM ((BYTE)1)
#else
#define ENDISM ((BYTE)0)
#endif

extern UCELL EP;
extern BYTE I;
extern CELL A;
extern CELL *M0;
extern UCELL MEMORY;
extern UCELL SP, RP;
extern UCELL S0, R0;
extern UCELL THROW;
extern UCELL BAD;
extern UCELL NOT_ADDRESS;
#define CHECKED 1       // address checking is mandatory in this implementation

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
int load_cell(UCELL addr, CELL *value);
int store_cell(UCELL addr, CELL value);
int load_byte(UCELL addr, BYTE *value);
int store_byte(UCELL addr, BYTE value);

int pre_dma(UCELL from, UCELL to);
int post_dma(UCELL from, UCELL to);

// Interface calls
uint8_t *native_address_of_range(UCELL addr, UCELL length);
CELL run(void);
CELL single_step(void);
int load_object(FILE *file, UCELL address);

// Additional implementation-specific routines, macros, types and quantities
int init(CELL *c_array, size_t size);
int register_args(int argc, const char *argv[]);

#define PACKAGE_UPPER_TRUE CELL_MASK            // VM TRUE flag
#define PACKAGE_UPPER_FALSE ((CELL)0)           // VM FALSE flag

#define CELL_W 4    // the width of a cell in bytes
#define POINTER_W (sizeof(void *) / CELL_W)   // the width of a machine pointer in cells

// A union to allow storage of machine pointers in VM memory
union _CELL_pointer {
    CELL cells[POINTER_W];
    void (*pointer)(void);
};
typedef union _CELL_pointer CELL_pointer;

#endif
