section .bss
	inta: resb 4
	floatb: resb 8
section .data
	sampleInt: db 5,0
	sampleFloat: db -5.2,0
	msgBoolean: db "Input: Enter a boolean value:", 10, 0
	inputBoolean: db "%d", 0
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
	call scanf
	pop rbp
	push rbp
	mov rdi, outputBooleanTrue
	call printf
	pop rbp
	push rbp
	mov rdi, outputFloat
	mov rsi, __float64__(1.3)
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
