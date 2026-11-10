assembly @blocked intel none 1 0 0 1 {
 operand obj<64x1> 0 -1 1 1 0 { alternative 1 0 -1 { group (zmm31:0) } }
 clobber xmm31
 text "vpxord zmm0,"
 use 0 0 0 0
 text ","
 use 0 0 0 0
}
function @blocked_user(%value : ptr) -> void {
 block ^entry:
  asm @blocked(%value)
  return void
}
