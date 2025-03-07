#include <math.h>

#define SIZE_BOOL_CONT sizeof(bool_cont)
typedef unsigned char bool_cont;

char rep[SIZE_BOOL_CONT * 8];

void bit_rep_init(void) {
    rep[(SIZE_BOOL_CONT * 8) - 1] = '\0';
}

int bit_bool(bool_cont cont, int position) {
    return cont & 1 << position;
}

void bit_assign(bool_cont *cont, int position, int val) {
    if ( (bit_bool(*cont, position) && !val) || (!bit_bool(*cont, position) && val) ) *cont ^= 1 << position;
}

char *bit_rep(bool_cont cont) {
    for (int i = 0; i < SIZE_BOOL_CONT * 8; i++) {
        if (bit_bool(cont, i)) rep[7 - i] = '1';
        else rep[7 - i] = '0';
    }
    return (char *) rep;
}