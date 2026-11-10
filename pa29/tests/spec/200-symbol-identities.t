declare global @assembly_external : i64
global @assembly_left : i64 = 17
global @assembly_right : i64 = 29
assembly @address intel none 1 0 0 2 {
  operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
  operand ptr -1 1 2 0 0 { alternative 1 0 -1 { group (r10:0) } }
  text "lea "
  use 1 0 0 0
  text ", [rip + "
  use 0 1 0 0
  text "]"
}
assembly @got_address intel read 1 0 0 2 {
  operand ptr 0 -1 1 0 0 { alternative 8 0 -1 {} }
  operand ptr -1 1 2 0 0 { alternative 1 0 -1 { group (r11:0) } }
  text "mov "
  use 1 0 0 0
  text ", [rip + "
  use 0 1 0 0
  text "@GOTPCREL]"
}
function @direct_symbols(%a : ptr, %b : ptr, %c : ptr, %d : ptr) -> void {
  block ^entry:
    asm @address(@assembly_left, %a)
    asm @address(@assembly_right, %b)
    %p = addr @assembly_left
    %next = index i64 %p, 1
    asm @address(%next, %c)
    asm @got_address(@assembly_external, %d)
    return void
}
