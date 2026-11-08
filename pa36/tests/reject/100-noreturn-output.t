assembly @stop intel none 33 0 0 1 {
  operand i64 -1 0 2 0 0 { alternative 1 0 -1 { group (rax) } }
  text "ud2"
}
function @invalid() -> void {
  slot $result : i64
  block ^entry:
    %p = addr $result
    asm @stop(%p)
    unreachable
}
