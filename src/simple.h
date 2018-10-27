#ifndef _SIMPLE_H_
#define _SIMPLE_H_

#include "asm.h"

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
//---------------  DEFINE / ENUM  ---------------
//-----------------------------------------------
//
#define TOKEN_SIZE  1024
#define GVAR_SIZE   255
#define ARGV_SIZE   10

enum {
    TOK_ID = 255,
    TOK_NUMBER,
    TOK_COMMENT,
    TOK_LABEL,
    TOK_PRE       // #
};
enum {
    TYPE_LONG = 0,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_POINTER,
    TYPE_STRUCT,
    TYPE_PSTRUCT,  // struct data *p;
    TYPE_UNKNOW
};
enum {
    FUNC_TYPE_NATIVE_C = 0,
    FUNC_TYPE_COMPILED,
    FUNC_TYPE_VM,
    FUNC_TYPE_MODULE     // .dll | .so
};

//-----------------------------------------------
//-------------------  STRUCT  ------------------
//-----------------------------------------------
//
typedef union  VALUE  VALUE;
typedef struct TVar   TVar;
typedef struct TFunc  TFunc;

union VALUE {
    long    l;  //: type integer
    float   f;  //: type float
    char    *s; //: type pointer of char
    void    *p; //: type pointer
};
struct TVar {
    char    *name;
    int     type;
    VALUE   value;
    void    *info;  // any information ... 
};
struct TFunc {
    char    *name;
    char    *proto;   // prototype
    UCHAR   *code;    // the function on JIT MODE | or VM in VM MODE
    int     type;     // FUNC_TYPE_NATIVE_C = 0, FUNC_TYPE_COMPILED, FUNC_TYPE_MODULE
    int     len;
    int     sub;  // used only in dynamic modules: .dll | .so
    TFunc   *next;
};


LIBIMPORT TVar Gvar [GVAR_SIZE];

LIBIMPORT void    CreateVarLong (char *name, long value);
LIBIMPORT TFunc * FuncFind      (char *name);
LIBIMPORT int     VarFind       (char *name);
LIBIMPORT int     RegFind       (char *name);

#ifdef __cplusplus
}
#endif
#endif // ! _SIMPLE_H_

