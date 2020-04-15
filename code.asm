section .data
	msgBoolean: db "Input: Enter a boolean value:", 10, 0
	inputBoolean: db "55", 0
	msgInt: db "Input: Enter an integer value:", 10, 0
	inputInt: db "52", 0
	msgFloat: db "Input: Enter a float value:", 10, 0
	inputFloat: db "0.000000",0
	outputBooleanTrue: db "Output: true", 10, 0,
	outputBooleanFalse: db "Output: false", 10, 0,
	outputInt: db "Output: 48", 10, 0,
	outputFloat: db "Output: 0.000000", 10, 0,
section .text
	global main
	extern scanf
	extern printf
