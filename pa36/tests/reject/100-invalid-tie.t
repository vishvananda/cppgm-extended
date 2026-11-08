assembly @bad intel none 1 0 0 2 {
  operand i64 0 -1 1 0 0 {
    alternative 1 0 -1 { group (rax:0) }
  }
  operand i64 1 -1 1 0 0 {
    alternative 0 0 0 { }
  }
  text "nop"
}
