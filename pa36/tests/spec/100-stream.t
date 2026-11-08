assembly @stream intel write 1 0 0 2 {
 operand ptr 0 -1 1 0 0 {
  alternative 1 0 -1 {
   group (r10:0)
  }
 }
 operand v128 1 -1 1 0 0 {
  alternative 1 0 -1 {
   group (xmm14:0)
  }
 }
 text "movntps ["
 use 0 0 0 0
 text "], "
 use 1 0 0 0
}
function @stream_ps(%p : ptr, %v : v128) -> void {
 block ^entry:
 asm @stream(%p, %v)
 return void
}
