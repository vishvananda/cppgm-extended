.globaltype group_value, i32
.section .text.group_read,"",@
.weak group_read
group_read:
 .functype group_read () -> (i32)
 global.get group_value
 end_function
.section .text.group_write,"",@
.weak group_write
group_write:
 .functype group_write (i32) -> ()
 local.get 0
 global.set group_value
 end_function
.globl group_value
group_value:
