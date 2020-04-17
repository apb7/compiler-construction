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
	 sub rsp, 96 
	 push rbp 
	 mov rdi, msgFloat 
	 call printf 
	 mov rdi, inputFloat 
	 mov rsi, RBP 
	 sub rsi, 0 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, [RBP - 0] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgFloat 
	 call printf 
	 mov rdi, inputFloat 
	 mov rsi, RBP 
	 sub rsi, 4 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, [RBP - 4] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgBoolean  
 	 call printf  
 	 mov rdi, inputBoolean 
	 mov rsi, RBP 
	 sub rsi, 8 
	 call scanf 
	 pop rbp 
	 push rbp 
	 cmp word[RBP - 8], 0 
	 jz boolPrintFalse14 
boolPrintTrue14: 
	 mov rdi, outputBooleanTrue 
	 jmp boolPrintEnd14 
boolPrintFalse14: 
	 mov rdi, outputBooleanFalse 
boolPrintEnd14: 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 9 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 9] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 11 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 11] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgInt 
	 call printf 
	 mov rdi, inputInt 
	 mov rsi, RBP 
	 sub rsi, 13 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputInt 
	 mov rsi, [RBP - 13] 
	 call printf 
	 pop rbp 
	 mov rsp, rbp
