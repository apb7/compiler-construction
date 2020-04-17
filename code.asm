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
	 mov rdi, msgFloat 
	 call printf 
	 mov rdi, inputFloat 
	 pop rbp 
	 push rbp 
	 mov rsi, [RBP - 8] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgFloat 
	 call printf 
	 mov rdi, inputFloat 
	 mov rsi, RBP 
	 sub rsi, 16 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, [RBP - 16] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgFloat 
	 call printf 
	 mov rdi, inputFloat 
	 mov rsi, RBP 
	 sub rsi, 24 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, [RBP - 24] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgBoolean  
 	 call printf  
 	 mov rdi, inputBoolean 
	 mov rsi, RBP 
	 sub rsi, 26 
	 call scanf 
	 pop rbp 
	 push rbp 
	 cmp word[RBP - 26], 0 
	 jz boolPrintFalse17 
boolPrintTrue17: 
	 mov rdi, outputBooleanTrue 
	 jmp boolPrintEnd17 
boolPrintFalse17: 
	 mov rdi, outputBooleanFalse 
boolPrintEnd17: 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 30 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 30] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 30 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 30] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 34 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 34] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, __float64__(1.3) 
	 movq xmm0, rsi  
	 mov rax, 1  
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputBooleanTrue 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputBooleanFalse 
	 call printf 
	 pop rbp 
	 mov rsp, rbp 
