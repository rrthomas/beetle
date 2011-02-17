/* INITIALT.C

    Vrsn  Date   Comment
    ----|-------|------------------------------------------------------------
    0.00 07nov94
    0.01 09nov94 Added test of initialisation code.
    0.02 30nov94 Changed message on completion for consistency with other
                 test programs, and made main give a return code.
    0.03 17feb95 Modified to work with new version of storage.c, and use
    	    	 btests.h rather than bintern.h.

    Reuben Thomas


    Test that the Beetle headers beetle.h and bportab.h compile properly, and
    that the storage allocation and register initialisation in storage.c
    works.

*/


#include <stdio.h>
#include <stdlib.h>
#include "beetle.h" 	/* main header */
#include "btests.h"	/* Beetle tests header */


int main(void)
{
    init_beetle((BYTE *)malloc(1024), 256, 16);

    printf("InitialT ran OK\n");
    return 0;
}
