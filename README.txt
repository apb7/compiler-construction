Use the following command in Linux terminal (Ubuntu 16.04) with gcc 5.4.0 and nasm 2.14.02 installed to run the final assembly code generated in code.asm:
    nasm -felf64 code.asm && gcc code.o && ./a.out

ASSUMPTIONS:

1. The iterator variable of FOR loop will have the value right bound + 1 after loop has terminated.

2. SWITCH statement cannot have (dynamic) arrays declared in any one of the cases because that array might be used in
    some other case (and the one with declaration might never run in the first place).  So, we do  not  know how much
    space to reserve on stack for this array.

3. If any output variable is assigned in any one of the SWITCH statement cases, it is assumed as assigned even though
    that case may not execute at runtime. Thus it is assumed that all the  output list variables are assigned  in the
    source code and this is not checked at runtime.

4. The variable bounds in dynamic arrays are assumed to be positive since for static arrays, the bounds are always NUM.