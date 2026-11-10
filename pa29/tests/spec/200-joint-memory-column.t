assembly @update att readwrite 1 0 1 3 {
  operand i32 0 1 3 1 0 {
    alternative 1 0 -1 { group (rax:0) }
    alternative 2 0 -1 { }
  }
  operand i32 2 -1 1 0 0 {
    alternative 4 0 -1 { range 0 1 0 0 31 0 }
    alternative 4 0 -1 { range 0 1 0 0 255 0 }
  }
  text "add"
  use 0 4 0 0
  text " "
  use 1 0 0 0
  text ", "
  use 0 0 0 0
}
function @update_memory(%object : ptr) -> void {
  block ^entry:
    asm @update(%object, %object, 200)
    return void
}
