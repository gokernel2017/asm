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
// Simple Assembler:
//
// FILE:
//   simple.c
//
//
// TO COMPILE IN 32 BITS:
//   gcc -c src/asm.c -m32 -Wall
//   gcc src/simple.c -o simple asm.o -m32 -Wall
//
//
// TO COMPILE IN 64 BITS:
//   gcc -c src/asm.c -m64 -Wall
//   gcc src/simple.c -o simple asm.o -m64 -Wall
//
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

#include "simple.h"

struct ARG {
    char  *string;
    char  text [50][50];
    int   tok [20];
    int   count;
} arg; // <<<<<<<<<<  GLOBAL  >>>>>>>>>>
struct DEFINE {
    char    *name;
    struct DEFINE *next;
}*Gdefine;

FILE  * fp = NULL;
ASM   * asm_function = NULL;
TFunc * Gfunc = NULL;  // store the user functions

static char *reg[] = { "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi", NULL };

TVar Gvar [GVAR_SIZE];

char  *str;
char  func_name [100];
char  label_name [100][100];

static int
    line,
    count,
    level,
    is_function,
    ifndef_true
    ;

//-----------------------------------------------
//-----------------  PROTOTYPES  ----------------
//-----------------------------------------------
//
void op_cmp (ASM *a);
void op_if  (ASM *a);
void op_mov (ASM *a);
void op_sub (ASM *a);
//
static void execute_call  (ASM *a, TFunc *func);
static void proc_function (void);
static void store_arg     (char *text);
static void DefineAdd     (char *name);
static void proc_ifdef    (char *name);
//
void lib_info (int i);
void lib_hello (void);
void lib_version (void);

TFunc stdlib[] = {
  //-----------------------------------------------------------------------
  // char*        char*     UCHAR*                int   int   int   TFunc*
  // name         proto     code                  type  len   sub   next
  //-----------------------------------------------------------------------
  { "info",       "0i",     (UCHAR*)lib_info,     0,    0,    0,    NULL },
  { "hello",      "00",     (UCHAR*)lib_hello,    0,    0,    0,    NULL },
  { "version",    "00",     (UCHAR*)lib_version,  0,    0,    0,    NULL },
  { NULL,         NULL,     NULL,                 0,    0,    0,    NULL }
};

int lex (void) {
    register char *p = arg.text[count]; //token;
    int pre = 0; // '#'

    *p = 0;

top:
    if (*str <= 32) {
        if (*str==0) return 0;
        str++;
        goto top;
    }

    //##########  WORD, IDENTIFIER ...  #########
    //
    if ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || *str == '%' || *str == '_' || *str == '#') {
        if (*str=='#') pre = 1;
        *p++ = *str++;
        while ((*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z') || (*str >= '0' && *str <= '9') || *str=='_') {
            *p++ = *str++;
        }
        *p = 0;

        if (pre) {
            arg.tok[count] = TOK_PRE;
            return TOK_PRE;
        }

        if (*str == ':') {
            arg.tok[count] = TOK_LABEL;
            return TOK_LABEL;
        }

        arg.tok[count] = TOK_ID;

        return TOK_ID;
    }

    //#################  NUMBER  ################
    //
    if (*str >= '0' && *str <= '9') {
        while ((*str >= '0' && *str <= '9') || *str == '.') {
            *p++ = *str++;
        }
        *p = 0;

        arg.tok[count] = TOK_NUMBER;

        return TOK_NUMBER;
    } 

    // comment:
    if (*str == '/' && str[1]=='/')
        return 0;

    *p++ = *str;
    *p = 0;
    arg.tok[count] = *str;
    return *str++;

    return 0;
}

void Assemble (ASM *a, char *text) {
    TFunc *func;

    store_arg (text);

    if (arg.count==0)
  return;

    if (arg.tok[0]==TOK_PRE && arg.count==1 && !strcmp(arg.text[0], "#endif")) {
        ifndef_true = 0;
  return;
    }

    if (ifndef_true == 1)
  return;

    if (arg.tok[0]==TOK_PRE && arg.count==2 && !strcmp(arg.text[0], "#ifdef")) {
        proc_ifdef (arg.text[1]);
  return;
    }

    if (arg.tok[0]==TOK_LABEL) {
        asm_label (a, arg.text[0]);
  return;
    }

    if (arg.tok[0]=='}' && level > 0) { // end if { }
        asm_label (a, label_name[level]);
        level--;
  return;
    }
    
    //-------------------------------------------
    // long var
    // function hello_world
    //-------------------------------------------
    //
    if (arg.count==2) {
        if (!strcmp(arg.text[0], "long")) {
            CreateVarLong (arg.text[1], 0);
            return;
        }
        if (!strcmp(arg.text[0], "function")) {
            is_function = 1;
            proc_function ();
            return;
        }
    }
    if (is_function==1 && !strcmp(arg.text[0], "end")) {
        is_function = 0;
  return;
    }

    if ((func = FuncFind(arg.text[0])) != NULL) {
        execute_call (a, func);
  return;
    }

    //-------------------------------------------
    // Assembly Opcode:
    //-------------------------------------------
    //
    if (!strcmp(arg.text[0], "cmp")) {
        op_cmp (a);
    }
    else if (!strcmp(arg.text[0], "if")) {
        op_if(a);
    }
    else if (!strcmp(arg.text[0], "jle") && arg.count==2) {
        emit_jump_jle (a, arg.text[1]);
    }
    else if (!strcmp(arg.text[0], "mov")) {
        op_mov (a);
    }
    else if (!strcmp(arg.text[0], "sub")) {
        op_sub(a);
    } else {
        Erro ("%d : Assemble Ilegal Identifier: '%s'\n",line, arg.string);
    }
}

static void execute_call (ASM *a, TFunc *func) {
    int count, i = 0, pos = 0, size = 4, var;

    for (count = 1; count < arg.count; count++) {

        if (arg.tok[count]==TOK_NUMBER) {
            if (i==0) {
                emit_function_arg1_long (a, atoi(arg.text[count]), 0);
            }
            else if (i==1) {
                emit_function_arg2_long (a, atoi(arg.text[count]), pos);
            }
            i++;
            pos += size;
            continue;
        }// if (arg.tok[count]==TOK_NUMBER)

        if ((var = VarFind(arg.text[count])) != -1) {
            if (Gvar[var].type != TYPE_FLOAT) {
                if (i==0) {
                    emit_function_arg1_var (a, &Gvar[var].value.l, 0);
                }
                else if (i==1) {
                    emit_function_arg2_var (a, &Gvar[var].value.l, pos);
                }
                i++;
                pos += size;
                continue;
            }
        }
    }
    
    // emit_call (a, func->code);

    if (func->type==FUNC_TYPE_MODULE) {
        emit_call_direct (a, func->code);
    } else {
        emit_call (a, func->code);
    }
}

void proc_ifdef (char *name) {
    struct DEFINE *p = Gdefine;
    while (p) {
        if (!strcmp(p->name, name)) {
            return;
        }
        p = p->next;
    }
    ifndef_true = 1;
}

//-------------------------------------------------------------------
//
// cmp  %eax , %ebx
//
//-------------------------------------------------------------------
//
void op_cmp (ASM *a) {
    if (arg.count==4) {
        if (!strcmp(arg.text[1], "%eax") && !strcmp(arg.text[3], "%ebx")) {
            emit_cmp_eax_ebx (a);
            return;
        } else {
            Erro ("%d: USAGE: cmp %eax, %ebx\n", line);
            return;
        }

    }
    Erro ("%d: '%s'\n", line, arg.string);
}

//-------------------------------------------------------------------
//
// if a > b {
//
// }
//
//-------------------------------------------------------------------
//
void op_if (ASM *a) {
    int var_a, var_b;
    if (arg.count == 5 && arg.tok[4]=='{') {
        static int level_count = 0;

        var_a = VarFind(arg.text[1]);
        var_b = VarFind(arg.text[3]);
        
        // mov   a, %ebx
        // mov   b, %eax

        if (var_a != -1) {
            emit_mov_var_reg (a, &Gvar[var_a].value.l, EBX);
        } else if (arg.tok[1]==TOK_NUMBER) {
            emit_mov_value_reg (a, atoi(arg.text[1]), EBX);
        } else {
            Erro ("%d: USAGE: if a > b {\n", line);
            return;
        }

        if (var_b != -1) {
            emit_mov_var_reg (a, &Gvar[var_b].value.l, EAX);
        } else if (arg.tok[3]==TOK_NUMBER) {
            emit_mov_value_reg (a, atoi(arg.text[3]), EAX);
        } else {
            Erro ("%d: USAGE: if a > b {\n", line);
            return;
        }

        level_count++;
        level++;
        sprintf (label_name[level], "block_%d_%d", level_count, level);

        if (arg.tok[2]=='>') {
            emit_cmp_eax_ebx (a);
            emit_jump_jle (a, label_name[level]);
            return;
        }
        if (arg.tok[2]=='<') {
            emit_cmp_eax_ebx (a);
            emit_jump_jge (a, label_name[level]);
            return;
        }
    }
    Erro ("%d: IF ERRO ''%s'\n", line, arg.string);
}

//-------------------------------------------------------------------
//
// mov  $ 1000 , var
// mov  $ 1000 , %eax
// mov  %eax , var
// mov  var , %eax
//
//-------------------------------------------------------------------
//
void op_mov (ASM *a) {
    int i;
    // mov $ 1000 , var
    // mov $ 1000 , %eax
    if (arg.count==5) {
        if (arg.tok[1]=='$' && arg.tok[2]==TOK_NUMBER && arg.tok[3]==',') {
            if (*arg.text[4]=='%') {
                if ((i = RegFind(arg.text[4])) != -1) {
                    emit_mov_value_reg (a, atoi(arg.text[2]), i);
                    return;
                }
            } else {
                if ((i = VarFind(arg.text[4])) != -1) {
                    emit_movl_var (a, atoi(arg.text[2]), &Gvar[i].value.l);
                    return;
                } else {
                    Erro ("%d: Elegal Identifier: '%s'\n", line, arg.text[4]);
                    return;
                }
            }
        }
    }// if (arg.count==5)

    // mov %eax , var
    // mov var , %eax
    if (arg.count==4) {
        int var;
        // mov %eax , var
        if (*arg.text[1]=='%') {
            if ((i = RegFind(arg.text[1])) != -1 && arg.tok[2]==',') {
                if ((var = VarFind(arg.text[3])) != -1) {
                    emit_mov_reg_var(a, i, &Gvar[var].value.l);
                    return;
                }
            }
        }
        // mov var , %eax
        if ((var = VarFind(arg.text[1])) != -1 && arg.tok[2]==',') {
            if ((i = RegFind(arg.text[3])) != -1) {
                emit_mov_var_reg(a, &Gvar[var].value.l, i);
                return;
            }
        }
    }// if (arg.count==4)

    Erro ("%d: '%s'\n", line, arg.string);
}

void op_sub (ASM *a) {
    //
    // sub    $ 8 , %esp
    // sub    $ 8 , %rsp
    //
    if (arg.count==5) {
        if (arg.tok[1]=='$' && arg.tok[2]==TOK_NUMBER && arg.tok[3]==',') {
            if (!strcmp(arg.text[4], "%esp") || !strcmp(arg.text[4], "%rsp")) {
                emit_sub_esp (a, atoi(arg.text[2]));
                return;
            }
        }
    }
    Erro ("%d: '%s'\n", line, arg.string);
}

static void store_arg (char *text) {
    arg.string = text;
    str = text;
    count = 0;
    while (lex()) { count++; }
    arg.count = count;
}

void Execute (ASM *a, char *FileName) {
    if ((fp = fopen(FileName, "rb")) != NULL) {
        char buf [255];

        emit_begin (a);
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            line++;
            Assemble (a, buf);
            if (erro) break;
        }
        emit_end (a);
        fclose(fp);

        if (asm_set_executable(a)==0) {
            Run_JIT (a);
        }
        else printf ("ERRO: %s\n", ErroGet());
    }
    else printf ("File Not Found: '%s'\n", FileName);
}

void proc_function (void) {
    char buf [255];
    TFunc *func;

    sprintf (func_name, "%s", arg.text[1]);

    if (FuncFind(func_name) != NULL) {
        Erro ("Function Exist(%s)\n", func_name);
  return;
    }

    asm_reset (asm_function);
    emit_begin (asm_function);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        line++;
        Assemble (asm_function, buf);
        if (is_function == 0) break;
        if (erro) break;
    }
    emit_end (asm_function);

    if ((func = (TFunc*)malloc (sizeof(TFunc))) == NULL)
  return;

    func->name = strdup (func_name);
    func->proto = strdup ("00");
    func->type = FUNC_TYPE_COMPILED;
    func->len = asm_get_len (asm_function);
    #ifdef WIN32
    if ((func->code = (UCHAR*)VirtualAlloc(NULL, func->len, MEM_COMMIT, PAGE_READWRITE)) == NULL) {
        Erro ("Create Function(%s) VirtualAlloc FAILED\n", func_name);
        return;
    }
    #endif
    #ifdef __linux__
    if ((func->code = (UCHAR*) malloc (func->len)) == NULL) {
    //if ((func->code = (UCHAR*)mmap (NULL, func->len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MMAP_ANON, -1, 0)) == MAP_FAILED) {
        Erro ("Create Function(%s) mmap FAILED\n", func_name);
        return;
    }
    #endif

    asm_code_copy (asm_function, func->code, func->len);

    Set_Executable (func->code, func->len);

    // add on top:
    func->next = Gfunc;
    Gfunc = func;
}

void CreateVarLong (char *name, long value) {
    TVar *v = Gvar;
    int i = 0;
    while (v->name) {
        if (!strcmp(v->name, name))
      return;
        v++;
        i++;
    }
    if (i < GVAR_SIZE) {
        v->name = strdup(name);
        v->type = TYPE_LONG;
        v->value.l = value;
        v->info = NULL;
    }
}

TFunc *FuncFind (char *name) {
    // array:
    TFunc *lib = stdlib;
    while (lib->name) {
        if ((lib->name[0]==name[0]) && !strcmp(lib->name, name))
      return lib;
        lib++;
    }
    // linked list:
    TFunc *func = Gfunc;
    while (func) {
        if ((func->name[0]==name[0]) && !strcmp(func->name, name))
      return func;
        func = func->next;
    }
    return NULL;
}

int VarFind (char *name) {
    TVar *v = Gvar;
    int i = 0;
    while(v->name) {
        if (!strcmp(v->name, name))
      return i;
        v++;
        i++;
    }
    return -1;
}
int RegFind (char *name) {
    char **p = reg;
    int i = 0;
    while (*p) {
        if (!strcmp(name, *p))
            return i;
        p++; i++;
    }
    return -1;
}

static void DefineAdd (char *name) {
    struct DEFINE *o = Gdefine, *n;
    while (o) {
        if (!strcmp(o->name, name)) return;
        o = o->next;
    }
    if ((n = (struct DEFINE*)malloc(sizeof(struct DEFINE)))==NULL) return;
    n->name = strdup(name);
    // add on top
    n->next = Gdefine;
    Gdefine = n;
}

void lib_hello (void) {
    printf ("\nFunction HELLO\n\n");
}

void lib_version (void) {
    printf ("__________________________\n");
    printf ("\nSimple Language:\n");

    #ifdef WIN32
    printf ("  Windows OS | WIN32\n");
    #endif
    #ifdef __linux__
    printf ("  Linux OS   | __linux__\n");
    #endif

    #if defined(__x86_64__)
    printf ("  64 BITS    | __x86_64__\n");
    #else
    printf ("  32 BITS    | __x86_32__\n");
    #endif

    printf ("  ASM JIT Version: %d.%d.%d\n", ASM_VERSION, ASM_VERSION_SUB, ASM_VERSION_PATCH);
    printf ("__________________________\n\n");
}

void lib_info (int i) {
    switch (i) {
    case 1: {
        TVar *v = Gvar;
        int i = 0;
        printf ("VARIABLES:\n---------------\n");
        while (v->name) {
            if (v->type==TYPE_LONG)   printf ("Gvar[%d](%s) = %ld\n", i, v->name, v->value.l);
            else
            if (v->type==TYPE_FLOAT) printf ("Gvar[%d](%s) = %f\n", i, v->name, v->value.f);
            v++; i++;
        }
        } break;

    default:
        printf ("USAGE(%d): info(1);\n\nInfo Options:\n 1: Variables\n 2: Functions\n 3: Defines\n 4: Words\n",i);
    }
}

int main (int argc, char **argv) {
    ASM *a;

    if ((a = asm_new (ASM_DEFAULT_SIZE)) == NULL)
  return -1;

    if ((asm_function = asm_new (ASM_DEFAULT_SIZE)) == NULL)
  return -1;

    #if defined(__x86_64__)
    DefineAdd ("__x86_64__");
    #else
    DefineAdd ("__x86_32__");
    #endif

    #ifdef WIN32
    DefineAdd ("WIN32");
    #endif
    #ifdef __linux__
    DefineAdd ("__linux__");
    #endif

    if (argc >= 2) {
        Execute (a, argv[1]);
        printf ("\nExiting With Sucess:\n");
    } else {
        printf ("USAGE: %s <file.asm>\n", argv[1]);
    }

    return 0;
}

