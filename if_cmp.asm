//-------------------------------------------------------------------
//
// OPCODE ( if / cmp ) Example:
//
// FILE:
//   if_cmp.asm
//
//-------------------------------------------------------------------
//
long a
long b

  mov   $200, a
  mov   $100, b
  

  if a > b {
    hello
  }

  //---------------------------------------------
  //
  // C Similar Code:
  //
  // if (a > b) {
  //     version();
  // }
  //
  //---------------------------------------------
  //
  mov   a, %ebx
  mov   b, %eax
  cmp   %eax, %ebx
  jle   label_pula  // >
    version
  label_pula:


