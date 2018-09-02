//-------------------------------------------------------------------
//
// Micro Assembler JIT:
//   Simple Test.
//
// FILE:
//   test.c
//
// COMPILE:
//   gcc test.c -o test libasm.a -O2 -Wall
//
//-------------------------------------------------------------------
//
#include "src/asm.h"

int var_a = 100;

void Hello (void) {
    printf ("\nFunction: Hello World\n\n");
}

int main (int argc, char **argv) {
    ASM *a;

    if ((a = asm_new (100)) == NULL)
  return -1;

    //-----------------------
    //
    emit_begin (a);
    emit_incl (a, &var_a);
    emit_end (a);
    //
    //-----------------------

    printf ("var_a: %d\n", var_a);

    if (SetExecutable (a, asm_get_len(a)) == 0) {
        Run (a);
    }
    else printf ("ERRO: %s\n", ErroGet());

    printf ("Exiting With Sucess | var_a: %d\n", var_a);

    return 0;
}
