assembly @raw intel none 1 0 0 1 {
 operand i64 0 -1 1 0 0 { alternative 1 0 -1 { group (rax) } }
 text "ret"
}
declare function @invalid() -> void
assembly_function @invalid {
 block ^entry:
  asm @raw(7)
  unreachable
}
