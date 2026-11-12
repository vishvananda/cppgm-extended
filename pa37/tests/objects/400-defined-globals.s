.globaltype base, i32, immutable
.globaltype derived, i32, immutable
.globaltype counter_value, i32
.section .text.base_declaration,"",@
.globl base_declaration
base_declaration:
 .functype base_declaration () -> (i32)
 global.get base
 end_function
.section .text.numeric_globals,"",@
.globl numeric_globals
numeric_globals:
 .functype numeric_globals (i32) -> (i32)
 global.get derived
 global.get base
 i32.sub
 global.get counter_value
 i32.add
 local.get 0
 i32.add
 global.set counter_value
 global.get counter_value
 end_function
.globl base
base:
.globl derived
derived:
.globl counter_value
counter_value:
.globaltype outside, i32
.globl outside
outside:
