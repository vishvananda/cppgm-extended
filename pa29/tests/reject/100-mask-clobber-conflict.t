assembly @impossible intel none 1 0 0 1 {
 operand i16 0 -1 1 0 0 { alternative 1 0 -1 { group (k0:0) group (k7:0) } }
 clobber k0
 clobber k7
 text "kmovw "
 use 0 0 0 0
 text ",k1"
}
function @conflict(%input : i16) -> void {
 block ^entry:
  asm @impossible(%input)
  return void
}
