//-------------------------------------------------------------------
//
// THANKS TO:
// ----------------------------------------------
//
//   01 : God the creator of the heavens and the earth in the name of Jesus Christ.
//
//   02 : Fabrice Bellard: www.bellard.org
//
// ----------------------------------------------
//
// Micro Assembler JIT (x86) 32/64 bits.
//
// FILE:
//   asm.h
//
// START DATE:
//   11/02/2013 - 17:20  ... Micro Assembler
//
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#ifndef _ASM_H_
#define _ASM_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
//---------------  DEFINE / ENUM  ---------------
//-----------------------------------------------
//
#define ASM_VERSION         0
#define ASM_VERSION_SUB     9
#define ASM_VERSION_PATCH   0
//
#define LIBIMPORT           extern
//
#define ASM_DEFAULT_SIZE    50000
#define UCHAR               unsigned char
#define MMAP_ANON           0x20
#define EMIT(a,op)          Emit(a, op, sizeof(op))
//
// OPCODES:
//
#define OP_NOP              0x90
#define OP_CALL             0xe8 // call direct

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
enum { // jump type:
    ASM_JUMP_JMP = 1,
    ASM_JUMP_JNE,
    ASM_JUMP_JLE,
    ASM_JUMP_JGE,
    ASM_JUMP_JG,
    ASM_JUMP_JE,
    ASM_JUMP_JL,
    ASM_JUMP_LOOP
};

//-----------------------------------------------
//-------------------  STRUCT  ------------------
//-----------------------------------------------
//
typedef struct ASM ASM; // opaque struct in file: "asm.c"


//-----------------------------------------------
//---------------  GLOBAL VARIABLE  -------------
//-----------------------------------------------
//
LIBIMPORT int erro;
//
// EMIT:
//
#define OPCODE LIBIMPORT const UCHAR
//
OPCODE MOV_EAX_EDI [2]; // mov   %eax, %edi
OPCODE MOV_EAX_ESI [2]; // mov   %eax, %esi
OPCODE MOV_EAX_EDX [2]; // mov   %eax, %edx
OPCODE MOV_EAX_ECX [2]; // mov   %eax, %ecx


//-------------------------------------------------------------------
//-------------------------  ASM PUBLIC API  ------------------------
//-------------------------------------------------------------------
//
LIBIMPORT ASM   * asm_new             (unsigned int size);
LIBIMPORT int     asm_set_executable  (ASM *a);
LIBIMPORT void    asm_get_addr        (ASM *a, void *ptr);
LIBIMPORT int     asm_get_len         (ASM *a);
LIBIMPORT void    asm_label           (ASM *a, char *name);
//
LIBIMPORT void    Emit                (ASM *a, const UCHAR opcode[], unsigned int len);
//
LIBIMPORT int     Set_Executable      (void *ptr, unsigned int size);
LIBIMPORT void    Run_JIT             (ASM *a);
// erro:
LIBIMPORT void    Erro                (const char *format, ...);
LIBIMPORT char  * ErroGet             (void);
LIBIMPORT void    ErroReset           (void);
//
// gen / emit:
//
LIBIMPORT void emit_begin         (ASM *a);
LIBIMPORT void emit_end           (ASM *a);
LIBIMPORT void emit_sub_esp       (ASM *a, char c);                 // sub    $32, %esp | sub  $32, %rsp
LIBIMPORT void emit_movl_ESP      (ASM *a, long value, char c);     // movl   $1000, 4(%esp)
LIBIMPORT void emit_mov_eax_ESP   (ASM *a, UCHAR index);            // mov    %eax, 4(%esp)
LIBIMPORT void emit_mov_value_reg (ASM *a, long value, int reg);    // mov    $1000, %eax
LIBIMPORT void emit_mov_var_reg   (ASM *a, void *var, int reg);     // move   variable to %register
LIBIMPORT void emit_mov_reg_var   (ASM *a, int reg, void *var);     // move   %register to variable
LIBIMPORT void emit_movl_var      (ASM *a, long value, void *var);  // movl   $1500, var
LIBIMPORT void emit_call          (ASM *a, void *func);
LIBIMPORT void emit_call_direct   (ASM *a, void *func);
//
LIBIMPORT void emit_cmp_eax_ebx   (ASM *a); // cmp  %eax, %ebx
// jump:
LIBIMPORT void emit_jump_jmp      (ASM *a, char *name);
LIBIMPORT void emit_jump_je       (ASM *a, char *name);
LIBIMPORT void emit_jump_jne      (ASM *a, char *name);
LIBIMPORT void emit_jump_jle      (ASM *a, char *name);
LIBIMPORT void emit_jump_jge      (ASM *a, char *name);
LIBIMPORT void emit_jump_jg       (ASM *a, char *name);
LIBIMPORT void emit_jump_jl       (ASM *a, char *name);
//
// Function Call Argument:
//
LIBIMPORT void emit_function_arg1_long  (ASM *a, long value, int pos);
LIBIMPORT void emit_function_arg2_long  (ASM *a, long value, int pos);
//
LIBIMPORT void emit_function_arg1_var   (ASM *a, void *var, int pos);
LIBIMPORT void emit_function_arg2_var   (ASM *a, void *var, int pos);

#ifdef __cplusplus
}
#endif
#endif // ! _ASM_H_

