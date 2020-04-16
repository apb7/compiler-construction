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
	mov rdi, msgBoolean
	call printf
	mov rdi, inputBoolean
	mov rsi, boolc
	call scanf
	pop rbp
	push rbp
	cmp word[boolc], 0 
	jz boolPrintFalse8 
boolPrintTrue8: 
	mov rdi, outputBooleanTrue 
	jmp boolPrintEnd8
boolPrintFalse8: 
	mov rdi, outputBooleanFalse 
boolPrintEnd8: 
	call printf 
	pop rbp
