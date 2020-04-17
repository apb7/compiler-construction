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
	sub rsp , 16

	push rbp
	mov rdi, outputInt
	mov rsi, rsp 
 	call printf  
	pop rbp 

	push rbp
	mov rdi, inputInt
	mov rsi, rbp
	sub rsi, 8
	call scanf
	pop rbp

	push rbp
	mov rdi, outputInt
	mov rsi, [rbp-8]
	call printf
	pop rbp

	push rbp
	mov rdi, outputInt
	mov rsi, rbp 
 	call printf  
	pop rbp 

	add rsp, 16


