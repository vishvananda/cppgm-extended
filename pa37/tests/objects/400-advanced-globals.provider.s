.functype global_target (i32) -> (i32)
.section .text.global_target,"",@
.globl global_target
global_target:
 .functype global_target (i32) -> (i32)
 local.get 0
 i32.const 17
 i32.add
 end_function

.globaltype seed, i32, immutable
.globl seed
seed:
.globaltype scaled, i32, immutable
.globl scaled
scaled:
.globaltype wide, i64, immutable
.globl wide
wide:
.globaltype single, i32, immutable
.globl single
single:
.globaltype double_value, i32, immutable
.globl double_value
double_value:
.globaltype lanes, i32, immutable
.globl lanes
lanes:
.globaltype callback, i32
.globl callback
callback:
.globaltype outside, i32
.globl outside
outside:
.globaltype counter, i32
.globl counter
counter:
