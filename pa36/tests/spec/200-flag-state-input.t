assembly @set_carry intel none 1 0 1 0 {
  text "stc"
}
assembly @consume_carry intel none 1 1 1 2 {
  operand i64 0 1 3 0 0 {
    alternative 1 0 -1 { group (r10:0) }
  }
  text "adc "
  use 0 0 0 0
  text ", 0"
}
function @carry_input(%out : ptr) -> void {
  block ^entry:
    asm @set_carry()
    asm @consume_carry(0, %out)
    return void
}
