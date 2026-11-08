assembly @bad intel none 1 0 0 1 {
  operand i64 0 -1 1 0 0 {
    alternative 1 0 -1 { group (rax:0) }
  }
  operand i32 -1 0 2 0 0 {
    alternative 1 0 -1 { group (rcx:0) }
  }
  text "nop"
}
