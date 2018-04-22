// Header for C Beetle containing all the data structures and interface
// calls specified in the definition of Beetle. This is the header file to
// include in programs using an embedded Beetle.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef BEETLE_BEETLE
#define BEETLE_BEETLE


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

// Beetle's registers

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
extern UCELL MEMORY;
extern UCELL SP, RP;
extern UCELL S0, R0;
extern UCELL HASHS, HASHR;
extern UCELL THROW;
extern UCELL BAD;
extern UCELL NOT_ADDRESS;
#define CHECKED 1       // C Beetle makes address checking mandatory

// Memory access

// Return value is 0 if OK, or exception code for invalid or unaligned address
int beetle_load_cell(UCELL addr, CELL *value);
int beetle_store_cell(UCELL addr, CELL value);
int beetle_load_byte(UCELL addr, BYTE *value);
int beetle_store_byte(UCELL addr, BYTE value);

int beetle_pre_dma(UCELL from, UCELL to, bool write);
int beetle_post_dma(UCELL from, UCELL to);

// Memory mapping
UCELL mem_here(void);
UCELL mem_allot(void *p, size_t n, bool writable);
UCELL mem_align(void);

// Interface calls
uint8_t *native_address(UCELL addr, bool writable);
CELL run(void);
CELL single_step(void);
int load_object(FILE *file, UCELL address);

// Additional routines, macros, types and quantities provided by C Beetle
int init_beetle(CELL *c_array, size_t size);
bool register_args(int argc, char *argv[]);

#define B_TRUE CELL_MASK            // Beetle's TRUE flag
#define B_FALSE ((CELL)0)           // Beetle's FALSE flag

#define CELL_W 4    // the width of a cell in bytes
#define POINTER_W (sizeof(void *) / CELL_W)   // the width of a machine pointer in cells

// A union to allow storage of machine pointers in Beetle's memory
typedef union {
    CELL cells[POINTER_W];
    void (*pointer)(void);
} CELL_pointer;


#endif
