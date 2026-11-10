declare function @module_typed_a(%value : i64) -> i64
declare function @module_typed_b(%value : i64) -> i64
assembly @definitions intel none 1 0 0 2 {
 operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
 operand ptr 1 -1 1 0 0 { alternative 8 0 -1 {} }
 text ".macro make_typed target.name:req,amount:req\n.globl \\target.name\n\\target.name:\nlea rax,[rdi+\\amount]\nret\n.endm\n.text\nMAKE_TYPED "
 use 0 1 0 0
 text ",11\nMAKE_TYPED "
 use 1 1 0 0
 text ",23\n"
}
assembly_global @definitions(@module_typed_a,@module_typed_b)
function @module_typed_caller(%value : i64) -> i64 {
 block ^entry:
  %a = call i64 @module_typed_a(%value)
  %b = call i64 @module_typed_b(%value)
  %result = binary add i64 %a,%b
  return i64 %result
}
