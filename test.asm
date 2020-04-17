section .bss
	 inta: resb 4
	 floatb: resb 8
	 boolc: resb 2
section .data
	 sampleInt: db 5,0
	 sampleFloat: db -5.2,0
	 msgBoolean: db "Input: Enter a boolean value:", 10, 0
	 inputBoolean: db "%hd", 0
	 msgInt: db "Input: Enter an integer value:", 10, 0
	 inputInt: db "%d", 0
	 msgFloat: db "Input: Enter a float value:", 10, 0
	 inputFloat: db "%lf",0
	 outputBooleanTrue: db "Output: true", 10, 0,
	 outputBooleanFalse: db "Output: false", 10, 0,
	 outputInt: db "Output: %d", 10, 0,
	 outputFloat: db "Output: %lf", 10, 0,

section .text
	 global main
	 extern scanf
	 extern printf

main:
	 mov rbp, rsp
	 mov rdx, rsp
	 sub rsp, 192
	 push rbp
	 mov rdi, msgInt
	 call printf
	 mov rdi, inputInt
	 mov rsi, RBP
	 sub rsi, 4
	 call scanf
	 pop rbp

	 mov rsi, rbp
	 sub rsi, 4
	 mov r8d, [rsi]
	 cmp r8d, 1
	 jz ptr_Num1
     cmp r8d, 2
     jz ptr_Num2
     jmp ptr_default
ptr_Num1:
	 push rbp
	 mov rdi, outputInt
	 mov rsi, [RBP - 4]
	 inc rsi
	 call printf
	 pop rbp
    jmp ID_ptr_break
ptr_Num2:
    	 push rbp
    	 mov rdi, outputInt
    	 mov rsi, [RBP - 4]
    	 dec rsi
    	 call printf
    	 pop rbp
    jmp ID_ptr_break
ptr_default:
	 push rbp
	 mov rdi, outputInt
	 mov rsi, [RBP - 4]
	 call printf
	 pop rbp
ID_ptr_break:
	 mov rsp, rbp
