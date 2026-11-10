assembly @change att readwrite 1 0 1 2 {
  operand i32 0 1 3 1 0 {
    alternative 2 0 -1 { }
    alternative 1 0 -1 { group (rax:0) }
  }
  text "addl $17, "
  use 0 0 0 0
}
function @change_value(%input : ptr, %output : ptr) -> void {
  block ^entry:
    asm @change(%input, %output)
    return void
}
