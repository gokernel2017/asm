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
// BY: Francisco - gokernel@hotmail.com
//
//-------------------------------------------------------------------
//
#include "simple.h"

typedef struct ARG ARG;
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

static char *reg[] = { "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi", NULL };

TVar Gvar [GVAR_SIZE];
char  *str;

static int
    line,
    count,
    ifndef_true
    ;

void op_mov (ASM *a);
void op_sub (ASM *a);
//
static void execute_call  (ASM *a, TFunc *func);
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
    if ((*str >= 'a' && *str <= 'z') || *str == '%' || *str == '_' || *str == '#') {
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

    if (ifndef_true == 1) {
  return;
    }

    if (arg.tok[0]==TOK_PRE && arg.count==2 && !strcmp(arg.text[0], "#ifdef")) {
        proc_ifdef (arg.text[1]);
  return;
    }

    if (arg.tok[0]==TOK_LABEL) {
        asm_label (a, arg.text[0]);
  return;
    }

    if (arg.count==2 && !strcmp(arg.text[0], "long")) {
        CreateVarLong (arg.text[1], 0);
  return;
    }

    if ((func = FuncFind(arg.text[0])) != NULL) {
        execute_call (a, func);
  return;
    }

    if (!strcmp(arg.text[0], "mov")) {
        op_mov (a);
    }
    else if (!strcmp(arg.text[0], "sub")) {
        op_sub(a);
    } else {
        Erro ("%d : Ilegal Identifier: '%s'\n",line, arg.string);
    }
}

static void execute_call (ASM *a, TFunc *func) {
    int count, i = 0, pos = 0, size = 4;

    for (count = 1; count < arg.count; count++) {

        if (arg.tok[count]==TOK_NUMBER) {
            if (i==0) {
                emit_function_arg1_value (a, atoi(arg.text[count]), 0);
            }
            else if (i==1) {
                emit_function_arg2_value (a, atoi(arg.text[count]), pos);
            }
            i++;
            pos += size;
        }// if (arg.tok[count]==TOK_NUMBER)
    }
    
    if (func->type==FUNC_TYPE_NATIVE_C) {
        emit_call (a, func->code);
    } else {
        emit_call_direct (a, func->code);
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

//
// mov $ 1000 , a
//
void op_mov (ASM *a) {
    int i;
    // mov $ 1000 , a
    // mov $ 1000 , %eax
    if (arg.count==5) {
        if (*arg.text[1]=='$' && arg.tok[2]==TOK_NUMBER && arg.tok[3]==',') {
            if (*arg.text[4]=='%') {
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
    }
    Erro ("%d: '%s'\n", line, arg.string);
}

void op_sub (ASM *a) {
    //
    // sub    $ 0x8 , %esp
    //
    if (arg.count==5) {
        if (*arg.text[1]=='$' && arg.tok[2]==TOK_NUMBER && arg.tok[3]==',') {
            if (!strcmp(arg.text[4], "%esp")) {
                emit_sub_esp (a, atoi(arg.text[2]));
            }
        }
    }
}

static void store_arg (char *text) {
    arg.string = text;
    str = text;
    count = 0;
    while (lex()) { count++; }
    arg.count = count;
}

void Execute (ASM *a, char *FileName) {
    FILE *fp;
    if ((fp = fopen(FileName, "rb")) != NULL) {
        char buf [255], *p;
        p = buf; *p = 0; //<<<<<<< set/reset
        emit_begin (a);
        while ((*p = getc(fp)) != EOF) {
            if (*p=='\n') {
                p--; *p = 0;
                line++;
                if (*buf) {
                    Assemble (a, buf);
                    if (erro) break;
                }
                p = buf; *p = 0; //<<<<<<< set/reset
            }
            else p++;
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
/*
    // linked list:
    TFunc *func = Gfunc;
    while (func) {
        if ((func->name[0]==name[0]) && !strcmp(func->name, name))
      return func;
        func = func->next;
    }
*/
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
    printf ("\n---------------------------");
    printf (
        "\nSimple Language:\n  ASM Version: %d.%d.%d\n",
        ASM_VERSION, ASM_VERSION_SUB, ASM_VERSION_PATCH
    );
    printf ("---------------------------\n");
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

    #if defined(__x86_64__)
    DefineAdd ("__x86_64__");
    #else
    DefineAdd ("__x86_32__");
    #endif

    if (argc >= 2) {
        Execute (a, argv[1]);
        printf ("Exiting With Sucess:\n");
    } else {
        printf ("USAGE: %s <file.asm>\n", argv[1]);
    }

    return 0;
}

