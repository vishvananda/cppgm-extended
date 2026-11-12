.globaltype shared_value, i32
.section .text.weak_read,"",@
.globl weak_read
weak_read:
 .functype weak_read () -> (i32)
 global.get shared_value
 end_function
.section .text.weak_write,"",@
.globl weak_write
weak_write:
 .functype weak_write (i32) -> ()
 local.get 0
 global.set shared_value
 end_function
.weak shared_value
shared_value:
