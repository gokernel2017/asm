//-------------------------------------------------------------------
//
// Micro Assembler JIT ( x86 ) 32/64 bits.
//
// FILE:
//   asm.c
//
// START DATE: 01/09/2018 - 07:30
//
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef WIN32
    #include <windows.h>
#endif
#ifdef __linux__
    #include <unistd.h>
    #include <sys/mman.h> // to: mprotect()
#endif

#include "asm.h"

#define STR_ERRO_SIZE   1024

//-----------------------------------------------
//------------------  STRUCT  -------------------
//-----------------------------------------------
//
typedef struct ASM_label  ASM_label;
typedef struct ASM_jump   ASM_jump;

struct ASM { // private struct
    UCHAR     *p;
    UCHAR     *code;
    ASM_label *label;
    ASM_jump  *jump;
    int       size;
};
struct ASM_label {
    char      *name;
    int       pos;
    ASM_label *next;
};
struct ASM_jump {
    char      *name;
    int       pos;
    int       type;
    ASM_jump  *next;
};

//-----------------------------------------------
//-----------------  VARIABLE  ------------------
//-----------------------------------------------
//
static char
    strErro [STR_ERRO_SIZE]
    ;
static int
    erro
    ;

void emit (ASM *a, const UCHAR opcode[], int len) {
    while (len--)
        *a->p++ = *opcode++;
}

void Erro (char *format, ...) {
    char msg[1024] = { 0 };
    va_list ap;

    va_start (ap,format);
    vsprintf (msg, format, ap);
    va_end (ap);

    erro++;
    if ((strlen(strErro) + strlen(msg)) < STR_ERRO_SIZE)
        strcat (strErro, msg);
}
char *ErroGet (void) {
    if (strErro[0])
        return strErro;
    else
        return NULL;
}
void ErroReset (void) {
    erro = 0;
    strErro[0] = 0;
}

ASM * asm_new (unsigned int size) {
    ASM *a = (ASM*)malloc(sizeof(ASM));
    if (a && (a->code=(UCHAR*)malloc(size)) != NULL) {
        a->p     = a->code;
        a->label = NULL;
        a->jump  = NULL;
        a->size  = size;
        return a;
    }
    return NULL;
}
static int asm_set_executable (void *ptr, unsigned int size) {

    if (ErroGet()) return 1;

#ifdef WIN32
    unsigned long old_protect;

    if (!VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &old_protect)) {
        Erro ("ERROR: asm_set_executable() ... NOT FOUND - VirtualProtect()\n");
        return 1; // erro
    }
#endif

#ifdef __linux__
    unsigned long start, end, PageSize;

    PageSize = sysconf (_SC_PAGESIZE);
    start = (unsigned long)ptr & ~(PageSize - 1);
    end = (unsigned long)ptr + size;
    end = (end + PageSize - 1) & ~(PageSize - 1);
    if (mprotect((void *)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        Erro ("ERROR: asm_set_executable() ... NOT FOUND - mprotec()\n");
        return 1; // erro
    }
#endif

    return 0; // no erro
}

static void asm_get_addr (ASM *a, void *ptr) { ///: 32/64 BITS OK
    *(void**)a->p = ptr;
    a->p += 4; // ! OK
}


int SetExecutable (ASM *a, unsigned int size) {
    return asm_set_executable (a->code, size);
}

void Run (ASM *a) {
    ( (void(*)()) a->code ) ();
}

int asm_get_len (ASM *a) {
    return (a->p - a->code);
}

//-------------------------------------------------------------------
//--------------------------  GEN / EMIT  ---------------------------
//-------------------------------------------------------------------
//
void g (ASM *a, UCHAR c) {
    *a->p++ = c;
}
void g2 (ASM *a, UCHAR c1, UCHAR c2) {
    a->p[0] = c1;
    a->p[1] = c2;
    a->p += 2;
}
void g3 (ASM *a, UCHAR c1, UCHAR c2, UCHAR c3) {
    a->p[0] = c1;
    a->p[1] = c2;
    a->p[2] = c3;
    a->p += 3;
}
void g4 (ASM *a, UCHAR c1, UCHAR c2, UCHAR c3, UCHAR c4) {
    a->p[0] = c1;
    a->p[1] = c2;
    a->p[2] = c3;
    a->p[3] = c4;
    a->p += 4;
}
void g5 (ASM *a, UCHAR c1, UCHAR c2, UCHAR c3, UCHAR c4, UCHAR c5) {
    a->p[0] = c1;
    a->p[1] = c2;
    a->p[2] = c3;
    a->p[3] = c4;
    a->p[4] = c5;
    a->p += 5;
}

void emit_begin (ASM *a) { ///: 32/64 BITS OK
    #if defined(__x86_64__)
    // 55         : push  %rbp
    // 48 89 e5   : mov   %rsp,%rbp
    //-----------------------------
    a->p[0] = 0x55;
    a->p[1] = 0x48;
    a->p[2] = 0x89;
    a->p[3] = 0xe5;
    a->p += 4;
    #else
    // 55     : push  %ebp
    // 89 e5  : mov   %esp,%ebp
    //-----------------------------
    a->p[0] = 0x55;
    a->p[1] = 0x89;
    a->p[2] = 0xe5;
    //
    // 83 ec 100  : sub  $100, %esp
    //-----------------------------
    a->p[3] = 0x83;
    a->p[4] = 0xec;
    a->p[5] = 100;
    a->p += 6;
    #endif
}
void emit_end (ASM *a) { ///: 32/64 BITS OK
    #if defined(__x86_64__)
    a->p[0] = 0xc9; // c9 : leaveq
    a->p[1] = 0xc3; // c3 : retq
    a->p += 2;
    #else
    a->p[0] = 0xc9; // c9 : leave
    a->p[1] = 0xc3; // c3 : ret
    a->p += 2;
    #endif
    if ((a->p - a->code) > a->size) {
        Erro ("ASM ERRO: code > size\n");
        return;
    }
//    asm_change_jump (a);
}
void emit_call (ASM *a, void *func, UCHAR arg_count, UCHAR return_type) {
    // b8   7a 13 40 00       mov    $0x40137a,%eax
    // ff d0                	call   *%eax
    //
    // ba   7a 13 40 00     mov $0x40137a,%edx
    // ff d2                call   *%edx
    g(a,0xba); asm_get_addr(a, func);
    g2(a,0xff,0xd2);
}
void emit_incl (ASM *a, void *var) { //: 32/64 BITS OK
    #if defined(__x86_64__)
    g3(a,0xff,0x04,0x25); asm_get_addr(a,var);  // ff 04 25   00 0a 60 00   : incl   0x600a00
    #else
    g2(a,0xff,0x05); asm_get_addr(a,var);       // ff 05      00 20 40 00   : incl   0x402000
    #endif
}

void emit_decl (ASM *a, void *var) { //: 32/64 BITS OK
    #if defined(__x86_64__)
    g3(a,0xff,0x0c,0x25); asm_get_addr(a, var);  // ff 0c 25   cc 0a 60 00   decl  0x600acc
    #else
    g2(a,0xff,0x0d); asm_get_addr(a, var);  // ff 0d      00 20 40 00   decl  0x402000
    #endif
}
void emit_mov_var_reg (ASM *a, void *var, int reg) { ///: 32/64 BITS OK: Move variable to %register
    if (reg >= 0 && reg <= 7) {
        #if defined(__x86_64__)
        switch (reg) {
        case EAX: g3(a,0x8b,0x04,0x25); break; // 8b 04 25   00 0a 60 00 	mov    0x600a00,%eax
        case ECX: g3(a,0x8b,0x0c,0x25); break; // 8b 0c 25   00 0a 60 00 	mov    0x600a00,%ecx
        case EDX: g3(a,0x8b,0x14,0x25); break; // 8b 14 25   00 0a 60 00 	mov    0x600a00,%edx
        case EBX: g3(a,0x8b,0x1c,0x25); break; // 8b 1c 25   00 0a 60 00 	mov    0x600a00,%ebx
        default: return;
        }
        asm_get_addr (a, var);
        #else
        switch (reg) {
        case EAX: g(a,0xa1);       break; // a1       60 40 40 00   mov   0x404060, %eax
        case ECX: g2(a,0x8b,0x0d); break;	// 8b 0d    70 40 40 00   mov   0x404070, %ecx
        case EDX: g2(a,0x8b,0x15); break; // 8b 15    70 40 40 00   mov   0x404070, %edx
        case EBX: g2(a,0x8b,0x1d); break; // 8b 1d    60 40 40 00   mov   0x404060, %ebx
        default: return;
        }
        asm_get_addr (a, var);
        #endif
    }
}

void emit_mov_reg_var (ASM *a, int reg, void *var) { ///: 32/64 BITS OK: Move %register to variable
    if (reg >= 0 && reg <= 7) {
        #if defined(__x86_64__)
        switch (reg) {
        case EAX: g3(a, 0x89, 0x04, 0x25);  break; // 89 04 25    28 0b 60 00 	mov  %eax, 0x600b28
        case ECX: g3(a, 0x89, 0x0c, 0x25);  break; // 89 0c 25    28 0b 60 00 	mov  %ecx, 0x600b28
        case EDX: g3(a, 0x89, 0x14, 0x25);  break; // 89 14 25    28 0b 60 00 	mov  %edx, 0x600b28
        case EBX: g3(a, 0x89, 0x1c, 0x25);  break; // 89 1c 25    28 0b 60 00 	mov  %ebx, 0x600b28
        default: return;
        }
        asm_get_addr(a,var);
        #else
        switch (reg) {
        case EAX: g(a,0xa3);       break; // a3       10 40 40 00   mov   %eax, 0x404010
        case ECX: g2(a,0x89,0x0d); break; // 89 0d    60 40 40 00   mov   %ecx, 0x404060
        case EDX: g2(a,0x89,0x15); break; // 89 15    60 40 40 00   mov   %edx, 0x404060
        case EBX: g2(a,0x89,0x1d); break;//  89 1d    60 40 40 00   mov   %ebx, 0x404060
        default: return;
        }
        asm_get_addr(a,var);
        #endif
    }
}
void emit_sub_esp (ASM *a, char c) { // 32/64 BITS OK
    #if defined(__x86_64__)
    g4(a,0x48,0x83,0xec,(char)c); // 48 83 ec   08   sub   $0x8,%rsp
    #else
    g3(a,0x83,0xec,(char)c);      // 83 ec      08   sub  $0x8,%esp
    #endif
}
