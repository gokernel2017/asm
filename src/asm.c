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
//   asm.c
//
// START DATE:
//   11/02/2013 - 17:20  ... Micro Assembler
//
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#ifdef WIN32
    #include <windows.h>
#endif
#ifdef __linux__
    #include <unistd.h>
    #include <sys/mman.h> // mprotect()
#endif

#include "asm.h"

#define STR_ERRO_SIZE   1024

typedef struct ASM_label	ASM_label;
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

static void asm_change_jump (ASM *a);

//-----------------------------------------------
//------------------  VARIABLES  ----------------
//-----------------------------------------------
//
static char strErro [STR_ERRO_SIZE];
//
// GLOBAL:
//
int erro;
//
// EMIT:
//
const UCHAR MOV_EAX_EDI [2] = { 0x89, 0xc7 };       // 89 c7      : mov   %eax, %edi
const UCHAR MOV_EAX_ESI [2] = { 0x89, 0xc6 };       // 89 c6      : mov   %eax, %esi
const UCHAR MOV_EAX_EDX [2] = { 0x89, 0xc2 };       // 89 c2      : mov   %eax, %edx
const UCHAR MOV_EAX_ECX [2] = { 0x89, 0xc1 };       // 89 c1      : mov   %eax, %ecx


ASM *asm_new (unsigned int size) {
		ASM *a = (ASM*)malloc(sizeof(ASM));
    if (a) {
    #ifdef WIN32
        if ((a->code = (UCHAR*)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE)) == NULL) {
            printf ("Erro(asm_new()): VirtualAlloc Not Found\n");
      return NULL;
        }
    #endif
    #ifdef __linux__
        if ((a->code = (UCHAR*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MMAP_ANON, -1, 0)) == MAP_FAILED) {
            printf ("Erro(asm_new()): mmap Not Found\n");
      return NULL;
        }
    #endif
        a->p     = a->code;
        a->label = NULL;
        a->jump  = NULL;
        a->size  = size;
        return a;
    }
    return NULL;
}

int asm_set_executable (ASM *a) {
    return Set_Executable (a->code, (a->p - a->code));
}

void asm_get_addr (ASM *a, void *ptr) { ///: 32/64 BITS OK
    *(void**)a->p = ptr;
    a->p += 4; // ! OK
}

int asm_get_len (ASM *a) {
    return (a->p - a->code);
}

void asm_label (ASM *a, char *name) {
    if (name) {
        ASM_label *lab;
        ASM_label *l = a->label;

        // find if exist:
        while (l) {
            if (!strcmp(l->name, name)) {
                printf ("Label Exist: '%s'\n", l->name);
                return;
            }
            l = l->next;
        }

        if ((lab = (ASM_label*)malloc (sizeof(ASM_label))) != NULL) {

            lab->name = strdup (name);
            lab->pos  = (a->p - a->code); // the index

            // add on top:
            lab->next = a->label;
            a->label = lab;
        }
    }
}


int Set_Executable (void *ptr, unsigned int size) {

    if (ErroGet()) return 1;

#ifdef WIN32
    unsigned long old_protect;

    if (!VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &old_protect)) {
        Erro ("ERROR: asm_set_executable() ... NOT FOUND - VirtualProtect()\n");
        return 1; // erro
    }
#endif
#ifdef __linux__
    if (mprotect((void *)ptr, size, PROT_READ | PROT_EXEC) == -1) {
        Erro ("ERROR: asm_set_executable() ... NOT FOUND - mprotec()\n");
        return 1; // erro
    }
#endif

    return 0; // no erro
}

void Emit (ASM *a, const UCHAR opcode[], unsigned int len) {
    while (len--)
        *a->p++ = *opcode++;
}

void Run_JIT (ASM *a) {
    ( (void(*)()) a->code ) ();
}

void Erro (const char *format, ...) {
    char msg[1024] = { 0 };
    va_list ap;

    va_start (ap,format);
    vsprintf (msg, format, ap);
    va_end (ap);
    if ((strlen(strErro) + strlen(msg)) < STR_ERRO_SIZE)
        strcat (strErro, msg);
    erro++;
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

//-------------------------------------------------------------------
//---------------------------  GEN / EMIT  --------------------------
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

void emit_begin (ASM *a) { // 32/64 BITS OK
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
    a->p += 3;
    #endif
}

void emit_end (ASM *a) { // 32/64 BITS OK
    #if defined(__x86_64__)
//    a->p[0] = 0x5d; // 5d : pop    %ebp
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
    asm_change_jump (a);
}

void emit_sub_esp (ASM *a, char c) { // 32/64 BITS OK
    #if defined(__x86_64__)
    g4(a,0x48,0x83,0xec,(char)c); // 48 83 ec   08   sub   $0x8,%rsp
    #else
    g3(a,0x83,0xec,(char)c);      // 83 ec      08   sub  $0x8,%esp
    #endif
}

void emit_mov_value_reg (ASM *a, long value, int reg) { // mov  $1000, %eax
    if (reg >= 0 && reg <= 7) {
        switch (reg) {
        case EAX: g(a,0xb8); break; // b8   e8 03 00 00   | mov  $0x3e8, %eax
        case ECX: g(a,0xb9); break; // b9   64 00 00 00   | mov  $0x64, %ecx
        case EDX: g(a,0xba); break; // ba   64 00 00 00   | mov  $0x64, %edx
        case EBX: g(a,0xbb); break; // bb   64 00 00 00   | mov  $0x64, %ebx
        //
        case ESI: g(a,0xbe); break; // be   e8 03 00 00   | mov  $0x3e8, %esi
        case EDI: g(a,0xbf); break; // bf   e8 03 00 00   | mov  $0x3e8, %edi
        default:
            Erro ("emit_mov_value_reg(...); Valid Register: EAX, ECX, EDX, EBX, ESI, EDI\n");
            return;
        }
        *(long*)a->p = value;
        a->p += 4;
    }
}

void emit_mov_var_reg (ASM *a, void *var, int reg) { // 32/64 BITS OK: Move variable to %register
#if defined(__x86_64__)
    switch (reg) {
    case EAX: g3(a,0x8b,0x04,0x25); break; // 8b 04 25  00 0a 60 00 	mov  0x600a00, %eax
    case ECX: g3(a,0x8b,0x0c,0x25); break; // 8b 0c 25  00 0a 60 00 	mov  0x600a00, %ecx
    case EDX: g3(a,0x8b,0x14,0x25); break; // 8b 14 25  00 0a 60 00 	mov  0x600a00, %edx
    case EBX: g3(a,0x8b,0x1c,0x25); break; // 8b 1c 25  00 0a 60 00 	mov  0x600a00, %ebx
    //
    case ESI: g3(a,0x8b,0x34,0x25); break; // 8b 34 25  24 7a 40 00   mov  0x407a24, %esi
    case EDI: g3(a,0x8b,0x3c,0x25); break; // 8b 3c 25  20 7a 40 00 	mov  0x407a20, %edi
    default:
        Erro ("emit_mov_var_reg(...); Valid Register: EAX, ECX, EDX, EBX, ESI, EDI\n");
        return;
    }
#else
    switch (reg) {
    case EAX: g(a,0xa1);       break; // a1       60 40 40 00   mov   0x404060, %eax
    case ECX: g2(a,0x8b,0x0d); break;	// 8b 0d    70 40 40 00   mov   0x404070, %ecx
    case EDX: g2(a,0x8b,0x15); break; // 8b 15    70 40 40 00   mov   0x404070, %edx
    case EBX: g2(a,0x8b,0x1d); break; // 8b 1d    60 40 40 00   mov   0x404060, %ebx
    //
    case ESI: g2(a,0x8b,0x35); break; // 8b 35    30 54 40 00   mov   0x405430, %esi
    case EDI: g2(a,0x8b,0x3d); break; // 8b 3d    34 54 40 00   mov   0x405434, %edi
    default:
        Erro ("emit_mov_var_reg(...); Valid Register: EAX, ECX, EDX, EBX, ESI, EDI\n");
        return;
    }
#endif
    asm_get_addr (a, var);
}

void emit_mov_reg_var (ASM *a, int reg, void *var) { // 32/64 BITS OK: Move %register to variable
#if defined(__x86_64__)
    switch (reg) {
    case EAX: g3(a,0x89,0x04,0x25); break; // 89 04 25    28 0b 60 00 	mov    %eax, 0x600b28
    case ECX: g3(a,0x89,0x0c,0x25); break; // 89 0c 25    28 0b 60 00 	mov    %ecx, 0x600b28
    case EDX: g3(a,0x89,0x14,0x25); break; // 89 14 25    28 0b 60 00 	mov    %edx, 0x600b28
    case EBX: g3(a,0x89,0x1c,0x25); break; // 89 1c 25    28 0b 60 00 	mov    %ebx, 0x600b28
    //
    case ESI: g3(a,0x89,0x34,0x25); break; // 89 34 25    24 7a 40 00 	mov    %esi, 0x407a24
    case EDI: g3(a,0x89,0x3c,0x25); break; // 89 3c 25    20 7a 40 00 	mov    %edi, 0x407a20
    default:
        Erro ("emit_mov_reg_var(...); Valid Register: EAX, ECX, EDX, EBX, ESI, EDI\n");
        return;
    }
#else
    switch (reg) {
    case EAX: g(a,0xa3);       break; // a3       10 40 40 00   mov   %eax, 0x404010
    case ECX: g2(a,0x89,0x0d); break; // 89 0d    60 40 40 00   mov   %ecx, 0x404060
    case EDX: g2(a,0x89,0x15); break; // 89 15    60 40 40 00   mov   %edx, 0x404060
    case EBX: g2(a,0x89,0x1d); break; // 89 1d    60 40 40 00   mov   %ebx, 0x404060
    //
    case ESI: g2(a,0x89,0x35); break; // 89 35    30 54 40 00   mov   %esi, 0x405430
    case EDI: g2(a,0x89,0x3d); break; // 89 3d    34 54 40 00   mov   %edi, 0x405434
    default:
        Erro ("emit_mov_reg_var(...); Valid Register: EAX, ECX, EDX, EBX, ESI, EDI\n");
        return;
    }
#endif
    asm_get_addr (a, var);
}

void emit_movl_ESP (ASM *a, long value, char c) { // movl  $1000, 4(%esp)
    #if defined(__x86_64__)
    // 64 bits:
    // 67 c7 44 24    04   d0 07 00 00  | movl  $0x7d0, 0x4(%esp)
    g4(a,0x67,0xc7,0x44,0x24); g(a,(char)c);
    *(long*)a->p = value;
    a->p += 4;
    #else
    // 32 bits:
    // c7 44 24   04    d0 07 00 00	  movl   $0x7d0,  0x4(%esp)
    g3(a,0xc7,0x44,0x24); g(a,(char)c);
    *(long*)a->p = value;
    a->p += 4;
    #endif
}

void emit_mov_eax_ESP (ASM *a, UCHAR index) {
    #if defined(__x86_64__)
    g5 (a,0x67,0x89,0x44,0x24, (UCHAR)index); // 67 89 44 24    04    mov   %eax, 4(%esp)
    #else
    g4 (a,0x89,0x44,0x24, (UCHAR)index); // 89 44 24     04    mov    %eax,0x4(%esp)
    #endif
}

void emit_movl_var (ASM *a, long value, void *var) {
    #if defined(__x86_64__)
    // 64 bits
    // c7 04 25     30 70 40 00     dc 05 00 00  	movl   $0x5dc,0x407030
    g3(a,0xc7,0x04,0x25);
    #else
    // 32 bits
    // c7 05    20 50 40 00    dc 05 00 00 	movl   $0x5dc,0x405020
    g2(a,0xc7,0x05);
    #endif
    asm_get_addr(a,var);
    *(long*)a->p = value;
    a->p += 4;
}

void emit_function_arg1_long (ASM *a, long value, int pos) {
#if defined(__x86_64__)
    #ifdef WIN32
    emit_mov_value_reg (a, value, ECX);
    #endif
    #ifdef __linux__
    emit_mov_value_reg (a, value, EDI);
    #endif
#else
    emit_movl_ESP (a, value, 0);
#endif
}
void emit_function_arg2_long (ASM *a, long value, int pos) {
#if defined(__x86_64__)
    #ifdef WIN32
    emit_mov_value_reg (a, value, EDX);
    #endif
    #ifdef __linux__
    emit_mov_value_reg (a, value, ESI);
    #endif
#else
    emit_movl_ESP (a, value, pos);
#endif
}

void emit_function_arg1_var (ASM *a, void *var, int pos) {
#if defined(__x86_64__)
    #ifdef WIN32
    emit_mov_var_reg (a, var, ECX);
    #endif
    #ifdef __linux__
    emit_mov_var_reg (a, var, EDI);
    #endif
#else
    emit_mov_var_reg (a, var, EAX);
    emit_mov_eax_ESP (a, 0);
#endif
}

void emit_function_arg2_var (ASM *a, void *var, int pos) {
#if defined(__x86_64__)
    #ifdef WIN32
    emit_mov_var_reg (a, var, EDX);
    #endif
    #ifdef __linux__
    emit_mov_var_reg (a, var, ESI);
    #endif
#else
    emit_mov_var_reg (a, var, EAX);
    emit_mov_eax_ESP (a, pos);
#endif
}

void emit_call (ASM *a, void *func) {
    // b8   7a 13 40 00       mov    $0x40137a,%eax
    // ff d0                	call   *%eax
    //
    g(a,0xb8); asm_get_addr(a, func);
    g2(a,0xff,0xd0);
}

void emit_call_direct (ASM *a, void *func) {
    //
    // Code based in this project:
    //
    // https://github.com/skeeto/dynamic-function-benchmark
    //
    // THANKS TO: Chris Wellons (  https://github.com/skeeto/ )
    //
    // uintptr_t rel = (uintptr_t)func - (uintptr_t)a->p - 5;
    //
    *a->p++ = OP_CALL; // 0xe8
    *(long*)a->p = (long*)func - (long*)(a->p - 4);
    a->p += 4;
}

void emit_cmp_eax_ebx (ASM *a) {
    g2(a,0x39,0xc3);   // 39 c3   cmp   %eax, %ebx
}

void emit_jump_jmp (ASM *a, char *name) {
    ASM_jump *jump;

    if (name && (jump = (ASM_jump*)malloc (sizeof(ASM_jump))) != NULL) {

#ifdef USE_ASM
write_asm(" jmp\t%s", name);
#endif

        g(a,OP_NOP); // change this in ( asm_change_jump ) to OPCODE: 0xe9

        jump->name = strdup (name);
        jump->pos  = (a->p - a->code); // the index
        jump->type = ASM_JUMP_JMP;

        // add on top:
        jump->next = a->jump;
        a->jump = jump;

        // to change ...
        g(a,OP_NOP); g(a,OP_NOP); g(a,OP_NOP); g(a,OP_NOP);
    }
}
//-------------------------------------------------------------------
// THANKS TO:
//
//   Fabrice Bellard: www.bellard.org
//
// CODE BASED:
// -------------------------------
//   LIB:  tcc-0.9.25
//   FILE: i386-gen.c
//   FUNC: void gjmp_addr (int a);
//   LINE: 568
// -------------------------------
//
// generate a jump to a fixed address
//
// NOTE: This jump is in the code[index]
//
//-------------------------------------------------------------------
//
static void asm_conditional_jump (ASM *a, char *name, int type) {
    ASM_jump *jump;

    if (name && (jump = (ASM_jump*)malloc (sizeof(ASM_jump))) != NULL) {
        jump->name = strdup (name);
        jump->pos  = (a->p - a->code); // the index
        jump->type = type;

        // add on top:
        jump->next = a->jump;
        a->jump = jump;

        // to change ...
        g(a,OP_NOP); g(a,OP_NOP);
        g(a,OP_NOP); g(a,OP_NOP); g(a,OP_NOP); g(a,OP_NOP);
    }
}

void emit_jump_je (ASM *a, char *name) {
    asm_conditional_jump (a, name, ASM_JUMP_JE);
}

void emit_jump_jne (ASM *a, char *name) {
    asm_conditional_jump (a, name, ASM_JUMP_JNE);
}

void emit_jump_jle (ASM *a, char *name) {
    asm_conditional_jump (a, name, ASM_JUMP_JLE);
}
void emit_jump_jge (ASM *a, char *name) {
    asm_conditional_jump (a, name, ASM_JUMP_JGE);
}
void emit_jump_jg (ASM *a, char *name) {
    asm_conditional_jump (a, name, ASM_JUMP_JG);
}
void emit_jump_jl (ASM *a, char *name) {
    asm_conditional_jump (a, name, ASM_JUMP_JL);
}

static void asm_change_jump (ASM *a) {
    ASM_label *label = a->label;

    while (label) {

        ASM_jump  *jump  = a->jump;

        while (jump) {

            if (!strcmp(label->name, jump->name)) {
                int jump_pos  = jump->pos;
                int label_pos = label->pos;

                switch (jump->type) {
                case ASM_JUMP_JMP:
                    {
                    *(UCHAR*)(a->code+jump_pos-1) = 0xe9; // OPCODE ( jmp )
                    *(int*)(a->code+jump_pos) = (int)(label_pos - jump_pos - 4);
                    }
                    break;

                case ASM_JUMP_JG:
                    {
                    int r = label_pos - jump_pos - 2;

                    if (r == (char)r) { // 2 bytes
                        // 7f 08                	 jg     4012b1 < _code + number >
                        //
                        *(UCHAR*)(a->code+jump_pos) = 0x7f;
                        *(UCHAR*)(a->code+jump_pos+1) = r;
                    } else { // 6 bytes
                        // 0f 8f    bb 00 00 00     jg     4013a7 < _code + number >
                        //
                        *(UCHAR*)(a->code+jump_pos) = 0x0f;
                        *(UCHAR*)(a->code+jump_pos+1) = 0x8f;
                        *(int*) (a->code+jump_pos+2) = (int)(label_pos - jump_pos - 6);
                    }
                    }
                    break;//: case ASM_JUMP_JG:
//------------------------------------------------------------

// 7c fe                	jl     401293 <label>
// 0f 8c    5a ff ff ff    	jl     401293 <label>
                case ASM_JUMP_JL:
                    {
                    int r = label_pos - jump_pos - 2;

                    if (r == (char)r) { // 2 bytes
                        //  7c fe     jl    40129e <_my_loop+0xe>
                        //
                        *(char*)(a->code+jump_pos) = 0x7c;
                        *(char*)(a->code+jump_pos+1) = r;
                    } else {
                        // 0f 8c    96 00 00 00    jl   40133e <_my_loop+0xa9>
                        //
                        *(char*)(a->code+jump_pos) = 0x0f;
                        *(char*)(a->code+jump_pos+1) = 0x8c;
                        *(int*) (a->code+jump_pos+2) = (int)(label_pos - jump_pos - 6);
                    }
                    }
                    break;

//------------------------------------------------------------

                case ASM_JUMP_JGE:
                    {
                    int r = label_pos - jump_pos - 2;
                    if (r == (char)r) { // 2 bytes
                        // 7d 06                	jge    40165f < code + number>
                        //
                        *(UCHAR*)(a->code+jump_pos) = 0x7d;
                        *(UCHAR*)(a->code+jump_pos+1) = r;
                    } else {
                        // 0f 8d   d3 00 00 00    	jge    401733 < code+ number>
                        //
                        *(UCHAR*)(a->code+jump_pos) = 0x0f;
                        *(UCHAR*)(a->code+jump_pos+1) = 0x8d;
                        *(int*) (a->code+jump_pos+2) = (int)(label_pos - jump_pos - 6);
                    }
                    }
                    break;//: case JUMP_TYPE_JGE:

                case ASM_JUMP_JLE:
                    {
                    int r = label_pos - jump_pos - 2;
                    if (r == (char)r) { // 2 bytes
                        // 7e 06                	jle    40165a < code + number >
                        //
                        *(UCHAR*)(a->code+jump_pos) = 0x7e;
                        *(UCHAR*)(a->code+jump_pos+1) = r;
                    } else {
                        // 0f 8e    d3 00 00 00    	jle    401733 < code + number >
                        //
                        *(UCHAR*)(a->code+jump_pos) = 0x0f;
                        *(UCHAR*)(a->code+jump_pos+1) = 0x8e;
                        *(int*) (a->code+jump_pos+2) = (int)(label_pos - jump_pos - 6);
                    }
                    } break;//: case JUMP_JUMP_JLE: {


                case ASM_JUMP_JE:
                    {
                    int r = label_pos - jump_pos - 2;
                    if (r == (char)r) { // 2 bytes
                        //  40129a:	74 02     je    40129e <_my_loop+0xe>
                        //
                        *(char*)(a->code+jump_pos) = 0x74;
                        *(char*)(a->code+jump_pos+1) = r;
                    } else {
                        // 4012a2:	0f 84    96 00 00 00    je   40133e <_my_loop+0xa9>
                        //
                        *(char*)(a->code+jump_pos) = 0x0f;
                        *(char*)(a->code+jump_pos+1) = 0x84;
                        *(int*) (a->code+jump_pos+2) = (int)(label_pos - jump_pos - 6);
                    }
                    }
                    break;


                case ASM_JUMP_JNE:
                    {
                    int r = label_pos - jump_pos - 2;

                    if (r == (char)r) { // 2 bytes
                        // 75 08                	 jne     4012b1 < _code + number >
                        //
                        *(char*)(a->code+jump_pos) = 0x75;
                        *(char*)(a->code+jump_pos+1) = r;
                    } else {
                        // 0f 85    85 ed bf ff   jne    401293 < _code + number >
                        //
                        *(char*)(a->code+jump_pos) = 0x0f;
                        *(char*)(a->code+jump_pos+1) = 0x85;
                        *(int*) (a->code+jump_pos+2) = (int)(label_pos - jump_pos - 6);
                    }
                    }
                    break;

                }//: switch (jump->type)

            }//: if (!strcmp(label->name, jump->name))

            jump = jump->next;

        }//: while (jump)

        label = label->next;

    }//: while (label)

}//: asm_change_jump ()


