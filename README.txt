Use the following command in linux terminal (Ubuntu 16.04) with gcc 5.4.0 and nasm 12.14.02 installed:
    nasm -felf64 code.asm && gcc code.o && ./a.out

ASSUMPTIONS:
1. (As of now) The iterator variable of FOR loop will have the value right bound + 1 after loop has terminated.
2. (Edit this!) SWITCH statement cannot have (dynamic) arrays declared in one of the cases. Because that array might be
    used in some other case and we do not know how much space to reserve on stack for this array.
3. If any output variable is assigned in any one of the cases it is assumed as assigned even though that case may not
    execute at runtime. Thus it is assumed that all the output list variables are assigned in the source code and this
    is not checked at runtime.






TODOs: (add here if any for combined tracking of all remaining work)

//TODO: make makefile submission ready
    $./compiler      testcase.txt       code.asm  should work at the console
//TODO: handle memory leaks
//TODO: Exhaustive testing of entire code

Note: Some of them may have been done long back just check and mark the `TODO` as `DONE` at the top of symbolTable.c.
      Also check the already present DONEs there. Ensure they are actually DONE and WORKING!

SEMANTIC ANALYZER:
//TODO: if dyn arrays allowed in input list : having a dynamic array in input list is no longer an error (regardless of indices being declared, treat as placeholders) in the same list. perform static checks (base type match and static bounds check)
//TODO: printf("Input source code is semantically correct and type checked..........\n"); if code passes all semantic and type checks
//TODO: printf("Code compiles successfully..........\n"); if code compiles successfully i.e. after codeGen
//TODO: destroy (free) the symbol table and other constructs (including AST) for continuous execution in driver. Every loop iteration must be a fresh start.
//TODO: sort errors based on line nos.
//TODO: traverse list for while stupid check has infinite loop for typeCheckTESTCASE.txt

TYPE CHECKER:
//TODO: handle TYPE ERRORs for array assignment a:=b where atleast one of a or b is a (partially) dynamic array.
//TODO: statements like
        a := arr[4];
        are generating TYPE ERROR.
        though, statements like
        a := arr[4] + 2;
        are not generating TYPE ERROR
        (a is integer variable, arr is Static integer array) check with other types of arrays too and see if typeCheckTESTCASE.txt compiles successfully



CODE GENERATION:
//(Assumption 1) TODO: FOR loop semantics in codeGen: iterator value restore or last terminating value?
//TODO: only 1 is true and only 0 means false in codeGen. Any other value is a runtime error for codeGen.
//TODO: dynamic arrays need not be the last declared variable in a module (currently they need to be).
//TODO: Take input of arrays in the following format:
        Input: Enter 6 array elements of integer type for range 10 to 15


TODOs In Files: (verify and delete from here and from the file)

ast.c:
    line 19
    line 976

symbolTable.h:
    line 24

symbolTable.c
    // The TODOs on top in this file are global and are written above
    // Those TODOs that are mentioned in between must be implemented already, verify and delete

codeGen.c
    // being worked on, delete this when all TODOs done

driver.c
    // being worked on, delete this when all TODOs done

error.c
    line 52

typeCheck.c:
    line 154
