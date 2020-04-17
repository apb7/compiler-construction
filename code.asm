section .bss 
	 inta: resb 2 
	 floatb: resb 8 
	 boolc: resb 2 
section .data 
	 sampleInt: db 5,0 
	 sampleFloat: db -5.2,0 
	 msgBoolean: db "Input: Enter a boolean value:", 10, 0 
	 inputBoolean: db "%hd", 0 
	 msgInt: db "Input: Enter an integer value:", 10, 0 
	 inputInt: db "%hd", 0 
	 msgFloat: db "Input: Enter a float value:", 10, 0 
	 inputFloat: db "%lf",0 
	 outputBooleanTrue: db "Output: true", 10, 0, 
	 outputBooleanFalse: db "Output: false", 10, 0, 
	 outputInt: db "Output: %hd", 10, 0, 
	 outputFloat: db "Output: %lf", 10, 0, 
 
section .text 
	 global main 
	 extern scanf 
	 extern printf 
 
main:
	 push rbp 
	 mov rdi, msgFloat 
	 call printf 
	 mov rdi, inputFloat 
	 mov rsi, floatb 
	 call scanf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, [floatb] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, outputFloat 
	 mov rsi, [floatb] 
	 call printf 
	 pop rbp 
	 push rbp 
	 mov rdi, msgBoolean  
 	 call printf  
 	 mov rdi, inputBoolean 
	 mov rsi, boolc 
	 call scanf 
	 pop rbp 
	 push rbp 
	 cmp word[boolc], 0 
	 jz boolPrintFalse12 
boolPrintTrue12: 
	 mov rdi, outputBooleanTrue 
	 jmp boolPrintEnd12 
boolPrintFalse12: 
	 mov rdi, outputBooleanFalse 
boolPrintEnd12: 
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
