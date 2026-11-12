.globaltype shared_value, i32
.section .text.strong_read,"",@
.globl strong_read
strong_read:
 .functype strong_read () -> (i32)
 global.get shared_value
 end_function
.globl shared_value
shared_value:
