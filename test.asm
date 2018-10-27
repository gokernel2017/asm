//-----------------------------------------------
//
// Simple Example:
//
// FILE:
//   test.asm
//
// USAGE:
//   simple test.asm
//
//-----------------------------------------------
//
long a
long b

#ifdef __x86_32__
  sub   $16, %esp
#endif

#ifdef __x86_64__
  hello
#endif

label_hello:

  mov   $1000, a
  mov   $1500, b

  // call function with argument: 1
  //
  info  1

  version

