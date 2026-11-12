.text
.globaltype __stack_pointer, i32
.tagtype unused_exception i64
.tagtype private_exception i32
.functype bridge_tag (i32) -> ()

.section .text.raise_tag,"",@
.globl raise_tag
raise_tag:
 .functype raise_tag (i32) -> ()
 local.get 0
 throw private_exception
 end_function

.section .text.catch_tag,"",@
.globl catch_tag
catch_tag:
 .functype catch_tag (i32) -> (i32)
 .local i32, i32
 global.get __stack_pointer
 local.set 1
 block i32
  try_table (catch private_exception 0)
   local.get 0
   call bridge_tag
   unreachable
  end_try_table
  unreachable
 end_block
 local.set 2
 local.get 1
 global.set __stack_pointer
 local.get 2
 end_function

.section .text.catch_reference,"",@
.globl catch_reference
catch_reference:
 .functype catch_reference (i32) -> (i32)
 .local i32, i32, exnref
 global.get __stack_pointer
 local.set 1
 block () -> (i32, exnref)
  try_table (catch_ref private_exception 0)
   local.get 0
   call bridge_tag
   unreachable
  end_try_table
  unreachable
 end_block
 local.set 3
 local.set 2
 local.get 1
 global.set __stack_pointer
 local.get 2
 local.get 3
 ref.null_exn
 i32.const 1
 select exnref
 ref.is_null
 i32.eqz
 i32.add
 end_function

.section .text.catch_any,"",@
.globl catch_any
catch_any:
 .functype catch_any (i32) -> (i32)
 .local i32
 global.get __stack_pointer
 local.set 1
 block
  try_table (catch_all 0)
   local.get 0
   call bridge_tag
   unreachable
  end_try_table
  unreachable
 end_block
 local.get 1
 global.set __stack_pointer
 i32.const 17
 end_function

.section .text.rethrow_tag,"",@
.globl rethrow_tag
rethrow_tag:
 .functype rethrow_tag (i32) -> (i32)
 .local i32, i32
 global.get __stack_pointer
 local.set 1
 block i32
  try_table (catch private_exception 0)
   block exnref
    try_table (catch_all_ref 0)
     local.get 0
     call bridge_tag
     unreachable
    end_try_table
    unreachable
   end_block
   throw_ref
  end_try_table
  unreachable
 end_block
 local.set 2
 local.get 1
 global.set __stack_pointer
 local.get 2
 end_function

.section .text.read_stack,"",@
.globl read_stack
read_stack:
 .functype read_stack () -> (i32)
 global.get __stack_pointer
 end_function

.section .text.null_reference,"",@
.globl null_reference
null_reference:
 .functype null_reference () -> (i32)
 ref.null_exn
 ref.is_null
 end_function

.globl unused_exception
unused_exception:
.globl private_exception
private_exception:
