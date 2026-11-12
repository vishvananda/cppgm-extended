.globaltype outside, externref
.section .text.null_global_value,"",@
.globl null_global_value
null_global_value:
 .functype null_global_value () -> (i32)
 global.get outside
 ref.is_null
 end_function
