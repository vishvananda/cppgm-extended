assembly @swap_constant intel none 1 0 1 3 {
  operand i64 -1 2 2 0 0 { alternative 1 1 -1 { group (r10:0) } }
  operand i64 0 -1 5 0 0 { alternative 1 0 -1 { group (r11:0) } }
  operand i64 1 -1 1 0 0 { alternative 4 0 -1 { range 0 0 5 0 5 0 } }
  text "mov "
  use 0 0 0 0
  text ", "
  use 1 0 0 0
  text "; add "
  use 0 0 0 0
  text ", "
  use 2 0 0 0
}
assembly @swap_storage intel read 1 0 1 3 {
  operand i64 -1 2 2 0 0 { alternative 1 1 -1 { group (r10:0) } }
  operand i64 0 -1 5 0 0 { alternative 2 0 -1 {} }
  operand i64 1 -1 1 1 0 { alternative 1 0 -1 { group (r11:0) } }
  text "mov "
  use 0 0 0 0
  text ", "
  use 1 0 0 0
  text "; add "
  use 0 0 0 0
  text ", "
  use 2 0 0 0
}
function @commutative_constant(%value : i64, %out : ptr) -> void {
  block ^entry:
    asm @swap_constant(5, %value, %out)
    return void
}
function @commutative_storage(%value : i64, %in : ptr, %out : ptr) -> void {
  block ^entry:
    asm @swap_storage(%value, %in, %out)
    return void
}
