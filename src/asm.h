//-------------------------------------------------------------------
//
// Micro Assembler JIT(x86) 32/64 bits.
//
// FILE:
//   asm.h
//
// START DATE: 01/09/2018 - 07:30
//
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#ifndef _ASM_H_
#define _ASM_H_

#include <stdio.h>

//-----------------------------------------------
//---------------  DEFINE / ENUM  ---------------
//-----------------------------------------------
//
#define LIBIMPORT         extern
//
#define EMIT(a,op)        emit(a, op, sizeof(op))
//
#define ASM_DEFAULT_SIZE  50000
#define UCHAR             unsigned char

#define G2_MOV_EAX_EDI 0x89, 0xc7 // mov   %eax, %edi
#define G2_MOV_EAX_ESI 0x89, 0xc6 // mov   %eax, %esi
#define G2_MOV_EAX_EDX 0x89, 0xc2 // mov   %eax, %edx
#define G2_MOV_EAX_ECX 0x89, 0xc1 // mov   %eax, %ecx
#define G3_MOV_EAX_r8d 0x41, 0x89, 0xc0 // mov   %eax, %r8d

enum {
    EAX = 0,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI
};


//-----------------------------------------------
//------------------  STRUCT  -------------------
//-----------------------------------------------
//
typedef struct ASM ASM; // opaque struct in file: "emit.c"


//-------------------------------------------------------------------
//--------------------------  PUBLIC API  ---------------------------
//-------------------------------------------------------------------
//
LIBIMPORT int     SetExecutable     (ASM *a, unsigned int size);
LIBIMPORT void    Run               (ASM *a);
LIBIMPORT void    Erro              (char *format, ...);
LIBIMPORT char  * ErroGet           (void);
LIBIMPORT void    ErroReset         (void);
//
LIBIMPORT ASM   * asm_new           (unsigned int size);
LIBIMPORT int     asm_get_len       (ASM *a);
//
// emit / gen:
//
LIBIMPORT void    emit              (ASM *a, const UCHAR opcode[], int len);
LIBIMPORT void    g                 (ASM *a, UCHAR c);
LIBIMPORT void    g2                (ASM *a, UCHAR c1, UCHAR c2);
LIBIMPORT void    g3                (ASM *a, UCHAR c1, UCHAR c2, UCHAR c3);
LIBIMPORT void    g4                (ASM *a, UCHAR c1, UCHAR c2, UCHAR c3, UCHAR c4);
LIBIMPORT void    g5                (ASM *a, UCHAR c1, UCHAR c2, UCHAR c3, UCHAR c4, UCHAR c5);
//
LIBIMPORT void    emit_begin        (ASM *a); // 32/64 BITS OK
LIBIMPORT void    emit_end          (ASM *a); // 32/64 BITS OK
//
LIBIMPORT void    emit_call         (ASM *a, void *func, UCHAR arg_count, UCHAR return_type); // 32/64 BITS OK
LIBIMPORT void    emit_incl         (ASM *a, void *var); // 32/64 BITS OK
LIBIMPORT void    emit_decl         (ASM *a, void *var); // 32/64 BITS OK
LIBIMPORT void    emit_mov_var_reg  (ASM *a, void *var, int reg); // 32/64 BITS OK: Move variable to %register
LIBIMPORT void    emit_mov_reg_var  (ASM *a, int reg, void *var); // 32/64 BITS OK: Move %register to variable
LIBIMPORT void    emit_sub_esp      (ASM *a, char c); // 32/64 BITS OK

#endif // ! _ASM_H_
