assembly @raw_add intel none 1 0 0 1 {
  operand i64 0 -1 1 0 0 { alternative 4 0 -1 {} }
  text "lea rax, [rdi + "
  use 0 1 0 0
  text "]; ret"
}
assembly @raw_tail att readwrite 1 0 7 1 {
  operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
  text "jmp "
  use 0 1 0 0
}
declare function @raw_add_23(%value : i64) -> i64
declare function @raw_add_41(%value : i64) -> i64
declare function @raw_external(%value : i64) -> i64
declare function @assembly_callback(%value : i64) -> i64
assembly_function @raw_add_23 {
  block ^entry:
    asm @raw_add(23)
    unreachable
}
assembly_function @raw_add_41 {
  block ^entry:
    asm @raw_add(41)
    unreachable
}
assembly_function @raw_external {
  block ^entry:
    asm @raw_tail(@assembly_callback)
    unreachable
}

assembly @raw_internal intel none 1 0 0 1 {
  operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
  text "lea rsi, [rdi + rdi]; jmp "
  use 0 1 0 0
}
function @internal_helper(%unused : i64, %value : i64) -> i64 [binding=internal] {
  block ^entry:
    %result = binary add i64 %value, 5
    return i64 %result
}
declare function @raw_to_internal(%value : i64) -> i64
assembly_function @raw_to_internal {
  block ^entry:
    asm @raw_internal(@internal_helper)
    unreachable
}
