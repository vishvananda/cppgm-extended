.globaltype seed, i32, immutable
.globaltype scaled, i32, immutable
.globaltype wide, i64, immutable
.globaltype single, f32, immutable
.globaltype double_value, f64, immutable
.globaltype lanes, v128, immutable
.globaltype callback, funcref
.globaltype outside, externref
.globaltype counter, i32
.tabletype __indirect_function_table, funcref
.functype global_target (i32) -> (i32)

.section .text.read_globals,"",@
.globl read_globals
read_globals:
 .functype read_globals () -> (i32)
 global.get scaled
 i32.const 51
 i32.eq
 global.get seed
 i32.const 17
 i32.eq
 i32.and
 global.get wide
 i64.const 1234605616436508552
 i64.eq
 i32.and
 global.get single
 f32.const 3.25
 f32.eq
 i32.and
 global.get double_value
 f64.const -7.5
 f64.eq
 i32.and
 global.get lanes
 i8x16.extract_lane_u 15
 i32.const 15
 i32.eq
 i32.and
 global.get outside
 ref.is_null
 i32.and
 end_function

.section .text.bump,"",@
.globl bump
bump:
 .functype bump (i32) -> (i32)
 global.get counter
 local.get 0
 i32.add
 global.set counter
 global.get counter
 end_function

.section .text.get_counter,"",@
.globl get_counter
get_counter:
 .functype get_counter () -> (i32)
 global.get counter
 end_function

.section .text.ref_roundtrip,"",@
.globl ref_roundtrip
ref_roundtrip:
 .functype ref_roundtrip (i32) -> (i32)
 global.get callback
 ref.null_func
 local.get 0
 select funcref
 ref.is_null
 local.get 0
 i32.eqz
 i32.eq
 if i32
  local.get 0
  i32.const global_target
  call_indirect (i32) -> (i32)
 else
  i32.const -99
 end_if
 end_function

.section .text.table_roundtrip,"",@
.globl table_roundtrip
table_roundtrip:
 .functype table_roundtrip () -> (i32)
 .local i32
 table.size __indirect_function_table
 local.set 0
 ref.null_func
 i32.const 2
 table.grow __indirect_function_table
 local.get 0
 i32.eq
 table.size __indirect_function_table
 local.get 0
 i32.const 2
 i32.add
 i32.eq
 i32.and
 local.get 0
 ref.func global_target
 i32.const 2
 table.fill __indirect_function_table
 local.get 0
 table.get __indirect_function_table
 ref.is_null
 i32.eqz
 i32.and
 local.get 0
 i32.const 1
 i32.add
 ref.null_func
 table.set __indirect_function_table
 local.get 0
 i32.const 1
 i32.add
 local.get 0
 i32.const 1
 table.copy __indirect_function_table, __indirect_function_table
 local.get 0
 i32.const 1
 i32.add
 table.get __indirect_function_table
 ref.is_null
 i32.eqz
 i32.and
 end_function

 .section .text.extern_identity,"",@
.globl extern_identity
extern_identity:
 .functype extern_identity (externref) -> (externref)
 local.get 0
 end_function
.section .text.func_identity,"",@
.globl func_identity
func_identity:
 .functype func_identity (funcref) -> (funcref)
 local.get 0
 end_function
.section .text.local_refs,"",@
.globl local_refs
local_refs:
 .functype local_refs (i32) -> (i32)
 ref.func global_target
 call func_identity
 ref.null_func
 local.get 0
 select funcref
 ref.is_null
 local.get 0
 i32.eqz
 i32.eq
 ref.null_extern
 call extern_identity
 ref.null_extern
 local.get 0
 select externref
 ref.is_null
 i32.and
 end_function
