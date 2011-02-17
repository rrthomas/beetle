/* Translation of Forth primes benchmark (adapted from Modula-2 version)
   For comparison against pForth
   Reuben Thomas   28/5/96
*/

#include <stdio.h>

#define size 400000
char isprime[size];

void do_prime()
{
    int i, j, k, prime, counter;

    counter = 0;
    for (j = 0; j < size; j++) {
        isprime[j] = 1;
    }

    for (j = 0; j < size; j++) {
        if (isprime[j]) {
            prime = 2*j+3;
            for (k = j+prime; k < size; k += prime) isprime[k] = 0;
            counter++;
        }
    }

    printf("%d primes ", counter);
}

void do_prime_hi()
{
    int i, j, k, prime, counter;

    counter = 0;
    for (j = 0; j < size; j++) {
        isprime[j] = 1;
    }

    for (j = 0; j < size; j++) {
        if (isprime[j]) {
            prime = 2*j+3;
            k = j+prime;
            if (k+j < size) for (; k < size; k += prime) isprime[k] = 0;
            counter++;
        }
    }

    printf("%d primes ", counter);
}

int main(void)
{
    int i;

    for (i = 0; i < 3; i++) do_prime(); putchar('\n');
    getchar();
    for (i = 0; i < 3; i++) do_prime_hi(); putchar('\n');

    return 0;
}
