	.globaltype __stack_pointer, i32
	.functype stack_position () -> (i32)
	.section .text.stack_position,"",@
	.globl stack_position
	.type stack_position,@function
stack_position:
	.functype stack_position () -> (i32)
	global.get __stack_pointer
	end_function
