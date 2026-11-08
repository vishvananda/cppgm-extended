assembly @branch intel none 1 0 1 3 {
  operand i64 0 1 3 0 0 { alternative 1 0 -1 { group (r10:0) } }
  operand i64 -1 2 2 0 0 { alternative 1 1 -1 { group (r11:0) } }
  text "mov "
  use 1 0 0 0
  text ", 0; cmp "
  use 0 0 0 0
  text ", 0; je .LUpper; 1: add "
  use 1 0 0 0
  text ", "
  use 0 0 0 0
  text "; sub "
  use 0 0 0 0
  text ", 1; jne 1b; jmp 2f; 1: add "
  use 1 0 0 0
  text ", 99; 2: .LUpper:"
}
function @direct_branch(%value : i64, %out : ptr) -> void {
  slot $used : i64
  block ^entry:
    %p = addr $used
    asm @branch(%value, %p, %out)
    asm @branch(%value, %p, %out)
    return void
}
