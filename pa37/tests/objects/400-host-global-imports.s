.globaltype imported_counter, i32
.import_module imported_counter, host
.import_name imported_counter, counter
.globaltype imported_bias, i64, immutable
.import_module imported_bias, host
.import_name imported_bias, bias
.section .text.touch_globals,"",@
.globl touch_globals
touch_globals:
 .functype touch_globals (i32) -> (i64)
 global.get imported_counter
 local.get 0
 i32.add
 global.set imported_counter
 global.get imported_bias
 global.get imported_counter
 i64.extend_i32_u
 i64.add
 end_function
