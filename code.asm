section .bss 
	 stack_top: resb 8 
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
	 mov QWORD[stack_top], rsp 
	 sub rsp, 192 
	 mov rsi, RBP 
	 mov rdi, rsi 
	 sub rdi, 14 
	 sub rsi, 18 
	 sub rsi, stack_top 
	 mov word[rdi], si 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 4 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 4] 
	 call printf 
	 pop rbp 
	 mov rsp, rbp 
