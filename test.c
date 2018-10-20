//-------------------------------------------------------------------
//
// Micro Assembler, Simple Example:
//
// COMPILE:
//   gcc -c src/asm.c -Wall
//   gcc test.c -o test asm.o -Wall
//
//-------------------------------------------------------------------
//
#include "src/asm.h"

void hello (void) {
    printf ("\nFUNCTION: Hello World\n\n");
}

int main (int argc, char **argv) {
    ASM *a;

    if ((a = asm_new(100))==NULL)
  return -1;

    emit_begin(a);
    emit_call (a, hello);
    emit_end  (a);

    if (asm_set_executable(a)==0) {
        Run_JIT(a);
    }
    else {
        printf ("Erro: %s\n", ErroGet());
        return -1;
    }

    printf ("Exiting With Sucess !!!\n");

    return 0;
}

