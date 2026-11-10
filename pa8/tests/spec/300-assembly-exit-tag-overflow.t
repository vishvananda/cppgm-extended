assembly @edge intel none 1 0 0 2 {
  operand i32 0 -1 1 0 0 { alternative 16 0 -1 {} }
  operand i32 -1 1 66 0 0 {}
}
function @rejected(%value : i32, %out : ptr) -> void {
  block ^entry:
    asm @edge(2147483648, %out)
    return void
}
