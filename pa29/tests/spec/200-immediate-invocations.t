assembly @literal intel none 1 0 0 2 {
  operand i64 -1 0 2 0 0 {
    alternative 1 0 -1 { group (rax:0) }
  }
  operand i64 1 -1 1 0 0 {
    alternative 4 0 -1 { }
  }
  text "mov "
  use 0 0 0 0
  text ", "
  use 1 0 0 0
}
function @first(%out : ptr) -> void {
  block ^entry:
    asm @literal(%out, 7)
    return void
}
function @second(%out : ptr) -> void {
  block ^entry:
    asm @literal(%out, 13)
    return void
}
function @both(%one : ptr, %two : ptr) -> void {
  block ^entry:
    asm @literal(%one, 23)
    asm @literal(%two, 31)
    return void
}
