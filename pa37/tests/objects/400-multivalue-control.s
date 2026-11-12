.text
.functype pair (i32) -> (i32, i64)
.section .text.pair,"",@
.globl pair
pair:
 .functype pair (i32) -> (i32, i64)
 local.get 0
 i64.const 9
 end_function

.section .text.evaluate,"",@
.globl evaluate
evaluate:
 .functype evaluate (i32) -> (i32)
 .local i32, i64
 local.get 0
 block (i32) -> (i32, i64)
  call pair
 end_block
 local.set 2
 local.set 1
 local.get 1
 local.get 2
 i32.wrap_i64
 i32.add
 end_function

.section .text.count_down,"",@
.globl count_down
count_down:
 .functype count_down (i32) -> (i32)
 .local i32, i32
 i32.const 0
 local.get 0
 loop (i32, i32) -> (i32)
  local.set 2
  local.set 1
  local.get 1
  local.get 2
  i32.add
  local.get 2
  i32.const 1
  i32.sub
  local.tee 2
  local.get 2
  i32.const 0
  i32.gt_s
  br_if 0
  drop
 end_loop
 end_function

.section .text.choose_tuple,"",@
.globl choose_tuple
choose_tuple:
 .functype choose_tuple (i32) -> (i32)
 .local i32, i64
 local.get 0
 if () -> (i32, i64)
  i32.const 3
  i64.const 8
 else
  i32.const 4
  i64.const 13
 end_if
 local.set 2
 local.set 1
 local.get 1
 local.get 2
 i32.wrap_i64
 i32.add
 end_function

.section .text.identity_if,"",@
.globl identity_if
identity_if:
 .functype identity_if (i32, i32) -> (i32)
 local.get 0
 local.get 1
 if (i32) -> (i32)
  i32.const 1
  i32.add
 end_if
 end_function

.section .text.table_exit,"",@
.globl table_exit
table_exit:
 .functype table_exit (i32) -> (i32)
 .local i32, i64
 block () -> (i32, i64)
  block () -> (i32, i64)
   i32.const 8
   i64.const 9
   local.get 0
   br_table {0, 1}
  end_block
  i64.const 1
  i64.add
 end_block
 local.set 2
 local.set 1
 local.get 1
 local.get 2
 i32.wrap_i64
 i32.add
 end_function

.section .text.polymorphic_dead,"",@
.globl polymorphic_dead
polymorphic_dead:
 .functype polymorphic_dead () -> (i32)
 block i32
  i32.const 17
  br 0
  i32.select
  drop
  unreachable
 end_block
 end_function

.section .text.wide_literal,"",@
.globl wide_literal
wide_literal:
 .functype wide_literal () -> (i32)
 i64.const -9223372036854775808
 i64.const 63
 i64.shr_s
 i64.const -1
 i64.eq
 end_function

.section .text.noop,"",@
.globl noop
noop:
 .functype noop () -> ()
 end_function
