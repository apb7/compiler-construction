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
	 output: db "Output: ", 0 
	 intHolder: db "%d ", 0 
	 newLine: db " ", 10, 0 
 
section .text 
	 global main 
	 extern scanf 
	 extern printf 
 
main: 
	 mov rbp, rsp 
	 mov QWORD[stack_top], rsp 
	 sub rsp, 192 

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

	 mov rdi, RBP 
	 sub rdi, 6 
	 mov rsi, rsp 
	 sub rsi, [stack_top] 
	 mov word[rdi], si 

		; push rdi
		; movsx rsi, word[rdi]
		; mov rdi, outputInt
		; call printf
		; pop rdi

	 sub rdi, 4 
	 mov esi, DWORD [RBP - 4] 
	 mov DWORD [rdi], esi 
	 sub rdi, 4 
	 mov DWORD [rdi], 8 
	 sub esi, 8 
	 inc esi 

		; push rbp
		; movsx rsi, esi
		; mov rdi, outputInt
		; call printf
		; pop rbp

	 sub rsp, 192 

	 mov rdi, msgInt 
	 	push rbp
	 call printf 
	 	
	 mov rcx, RBP 
	 sub rcx, 6 
	 movsx rsi, word[rcx] 
	 add rsi, stack_top 
	 mov rdi, inputInt 
	 sub rcx, 4 
	 movsx r12, DWORD [rcx] 


statarr_0x559fe18354a0: 
	 mov rdi, inputInt 

	 push rsi 
	 push rcx 
	 call scanf 
	 pop rcx 
	 pop rsi 

	 	 push rsi 
		 push rcx 
 		 mov rdi, outputInt
 		 mov rsi, r12
		 call printf 
		 pop rcx 
		 pop rsi
	 	

	 cmp r12, 8 
	 jz statarrExit_0x559fe18354a0 
	 inc r12 
	 sub rsi, -4 
	 jmp statarr_0x559fe18354a0 
statarrExit_0x559fe18354a0: 
		pop rbp
	 mov rsp, rbp 
