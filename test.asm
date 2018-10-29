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

function version_set_var
  version

  mov   $1, %eax
  mov   $1500, %ebx

  mov   %eax, a
  mov   %ebx, b
end

#ifdef __x86_32__
  sub   $32, %esp
#endif

  version_set_var

  // call function with argument: 1
  //
  info  a

