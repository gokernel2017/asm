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

//  mov   $1, a
//  mov   $1500, b

  mov   $1, %eax
  mov   $1500, %edx

  mov   %eax, a
  mov   %edx, b

  // call function with argument: 1
  //
  info  a

  version

