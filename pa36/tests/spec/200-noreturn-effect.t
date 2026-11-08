assembly @stop intel write 33 0 0 1 {
  operand ptr 0 -1 1 0 0 { alternative 1 0 -1 { group (rdi) } }
  text "mov dword ptr ["
  use 0 0 0 0
  text "], 77; ud2"
}
function @asm_stop(%state : ptr) -> void {
  block ^entry:
    asm @stop(%state)
    unreachable
}

assembly @raw_return intel none 33 0 0 0 {
  text "lea rax, [rdi + 7]; ret"
}
declare function @raw_flagged(%value : i64) -> i64
assembly_function @raw_flagged {
  block ^entry:
    asm @raw_return()
    unreachable
}
function @raw_flagged_caller(%value : i64) -> i64 {
  block ^entry:
    %result = call i64 @raw_flagged(%value)
    %next = binary add i64 %result, 5
    return i64 %next
}
